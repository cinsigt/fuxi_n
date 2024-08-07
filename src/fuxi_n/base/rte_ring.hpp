
#include "fuxi/base/rte_ring.h"

#include <strings.h>

#include "absl/numeric/bits.h"
#include "fuxi/common/log.h"

namespace next {
namespace fuxi {

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) !!(x)
#define unlikely(x) !!(x)
#endif

template <typename T>
RteRing<T>::RteRing() {}

template <typename T>
RteRing<T>::RteRing(uint32_t object_num, bool is_sp, bool is_sc) {
  ::next::fuxi::PlacementStorage storage;
  Init(object_num, is_sp, is_sc, storage);
}

template <typename T>
RteRing<T>::RteRing(uint32_t object_num, bool is_sp, bool is_sc, const ::next::fuxi::PlacementStorage& placement_storage) {
  Init(object_num, is_sp, is_sc, placement_storage);
}

template <typename T>
RteRing<T>::~RteRing() {
  if ((use_outer_buffer_ == false) && storage_) {
    ::free(storage_);
    storage_ = nullptr;
  }
}

template <typename T>
/*static*/ size_t RteRing<T>::AlignmentSize() {
  constexpr size_t alignment = 128;
  return alignment;
}

template<typename T>
/*static*/ size_t RteRing<T>::AlignedNum(size_t num) {
  return ::absl::bit_ceil(num);
}

template<typename T>
/*static*/ size_t RteRing<T>::PlacementStorageSize(size_t num) {
  size_t alignment = AlignmentSize();
  size_t size = AlignedNum(num);
  size_t buffer_size =
      (size * sizeof(T) + alignment - 1) & static_cast<size_t>(-alignment);
  return buffer_size;

}

template <typename T>
bool RteRing<T>::Init(uint32_t object_num, bool is_sp, bool is_sc) {
  ::next::fuxi::PlacementStorage storage;
  return Init(object_num, is_sp, is_sc, storage);
}

template <typename T>
bool RteRing<T>::Init(uint32_t object_num, bool is_sp, bool is_sc, const ::next::fuxi::PlacementStorage& placement_storage) {
  if (inited_) {
    return true;
  }

  is_sp_ = is_sp;
  is_sc_ = is_sc;
  size_ = AlignedNum(object_num);
  capacity_ = size_;
  mask_ = size_ - 1;

  size_t buffer_size = PlacementStorageSize(object_num);
  // adapt data storage field
  if ((placement_storage.FieldSize() == 1) && placement_storage.GetFieldBuffer(0).ptr) {
    storage_ = reinterpret_cast<T*>(placement_storage.GetFieldBuffer(0).ptr);
    use_outer_buffer_ = true;
  } else {
    storage_ = (T*)::aligned_alloc(AlignmentSize(), buffer_size);
  }
  bzero(storage_, buffer_size);

  inited_ = true;
  // AINFO << "init ring_buff min size " << object_num << " size " << size_ << "
  // is_sp " << is_sp_
  //        << " is_sc " << is_sc_;
  return true;
}

// check cons tail
template <typename T>
uint32_t RteRing<T>::MoveProdHead(uint32_t n, uint32_t* old_head,
                                  uint32_t* new_head, uint32_t* free_entries,
                                  bool is_sp) {
  uint32_t cons_tail;
  uint32_t max = n;
  bool success;

  *old_head = prod_.head.load(::std::memory_order_relaxed);
  do {
    n = max;
    std::atomic_thread_fence(::std::memory_order_acquire);

    cons_tail = cons_.tail.load(::std::memory_order_acquire);

    *free_entries = (capacity_ + cons_tail - *old_head);
    if (unlikely(n > *free_entries)) {
      // fixed size
      n = 0;
    }
    if (n == 0) {
      return 0;
    }

    *new_head = *old_head + n;
    if (is_sp) {
      prod_.head = *new_head, success = 1;
    } else {
      success = prod_.head.compare_exchange_strong(*old_head, *new_head,
                                                   ::std::memory_order_relaxed,
                                                   ::std::memory_order_relaxed);
    }
  } while (unlikely(success == 0));
  return n;
}

// check prod tail
template <typename T>
uint32_t RteRing<T>::MoveConsHead(uint32_t n, uint32_t* old_head,
                                  uint32_t* new_head, uint32_t* entries,
                                  bool is_sc) {
  uint32_t max = n;
  uint32_t prod_tail;
  bool success;

  *old_head = cons_.head.load(::std::memory_order_relaxed);
  do {
    n = max;
    std::atomic_thread_fence(::std::memory_order_acquire);

    // 生产者下界
    prod_tail = prod_.tail.load(::std::memory_order_acquire);
    *entries = (prod_tail - *old_head);

    if (n > *entries) {
      // fixed size
      n = 0;
    }
    if (unlikely(n == 0)) {
      return 0;
    }

    *new_head = *old_head + n;
    if (is_sc) {
      cons_.head = *new_head, success = 1;
    } else {
      success = cons_.head.compare_exchange_strong(*old_head, *new_head,
                                                   ::std::memory_order_relaxed,
                                                   ::std::memory_order_relaxed);
    }
  } while (unlikely(success == 0));
  return n;
}

template <typename T>
uint32_t RteRing<T>::DoEnqueue(T const* obj_table, uint32_t n,
                               uint32_t* free_space, bool is_sp) {
  uint32_t prod_head, prod_next;
  uint32_t free_entries;

  // get range [prod_head, prod_next)
  n = MoveProdHead(n, &prod_head, &prod_next, &free_entries, is_sp);
  if (n == 0) {
    goto end;
  }

  EnqueueValues(storage_, prod_head, obj_table, n);
  UpdateTail(&prod_, prod_head, prod_next, is_sp);

end:
  if (free_space != nullptr) {
    *free_space = free_entries - n;
  }
  return n;
}

template<typename T>
uint32_t RteRing<T>::DoDequeue(T* obj_table, uint32_t n,
                               uint32_t* available, bool is_sc) {
  uint32_t cons_head, cons_next;
  uint32_t entries;

  // get range [cons_head, cons_next)
  n = MoveConsHead(n, &cons_head, &cons_next, &entries, is_sc);
  if (n == 0) {
    goto end;
  }

  DequeueValues(storage_, cons_head, obj_table, n);
  UpdateTail(&cons_, cons_head, cons_next, is_sc);

end:
  if (available != nullptr) {
    *available = entries - n;
  }
  return n;
}

template <typename T>
int RteRing<T>::EnqueueValues(T* ring_start, uint32_t prod_head,
                            T const* obj_table, uint32_t n) {
  uint32_t i;
  const uint32_t size = size_;
  uint32_t idx = prod_head & mask_;
  T* ring = ring_start;

  if (likely(idx + n < size)) {
    for (i = 0; i < (n & ((~(unsigned)0x3))); i += 4, idx += 4) {
      ring[idx] = obj_table[i];
      ring[idx + 1] = obj_table[i + 1];
      ring[idx + 2] = obj_table[i + 2];
      ring[idx + 3] = obj_table[i + 3];
    }

    switch (n & 0x3) {
      case 3:
        ring[idx++] = obj_table[i++];  // fallthrough
      case 2:
        ring[idx++] = obj_table[i++];  // fallthrough
      case 1:
        ring[idx++] = obj_table[i++];
    }
  } else {
    for (i = 0; idx < size; i++, idx++) {
      ring[idx] = obj_table[i];
    }
    for (idx = 0; i < n; i++, idx++) {
      ring[idx] = obj_table[i];
    }
  }
  return 0;
}

template <typename T>
int RteRing<T>::DequeueValues(T* ring_start, uint32_t cons_head,
                            T* obj_table, uint32_t n) {
  uint32_t i;
  uint32_t idx = cons_head & mask_;
  const uint32_t size = size_;
  T* ring = ring_start;
  if (likely(idx + n < size)) {
    for (i = 0; i < (n & (~(unsigned)0x3)); i += 4, idx += 4) {
      obj_table[i] = ring[idx];
      obj_table[i + 1] = ring[idx + 1];
      obj_table[i + 2] = ring[idx + 2];
      obj_table[i + 3] = ring[idx + 3];
    }
    switch (n & 0x3) {
      case 3:
        obj_table[i++] = ring[idx++];  // fallthrough
      case 2:
        obj_table[i++] = ring[idx++];  // fallthrough
      case 1:
        obj_table[i++] = ring[idx++];
    }
  } else {
    for (i = 0; idx < size; i++, idx++) {
      obj_table[i] = ring[idx];
    }
    for (idx = 0; i < n; i++, idx++) {
      obj_table[i] = ring[idx];
    }
  }
  return 0;
}

}  // namespace fuxi
}  // namespace next
