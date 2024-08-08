
#include "fuxi/base/ring_buffer.h"

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

RingBuffer::RingBuffer() {}

RingBuffer::RingBuffer(uint32_t min_size, bool is_sp, bool is_sc) {
  Init(min_size, is_sp, is_sc);
}

RingBuffer::~RingBuffer() {
  if (ring_) {
    ::free(ring_);
    ring_ = nullptr;
  }
}

bool RingBuffer::Init(uint32_t min_size, bool is_sp, bool is_sc) {
  if (inited_) {
    return true;
  }

  is_sp_ = is_sp;
  is_sc_ = is_sc;
  size_ = ::absl::bit_ceil(min_size);
  capacity_ = size_;
  mask_ = size_ - 1;

  constexpr size_t alignment = 128;
  size_t buffer_size =
      (size_ * sizeof(void*) + alignment - 1) & static_cast<size_t>(-alignment);
  ring_ = (void**)::aligned_alloc(alignment, buffer_size);
  bzero(ring_, buffer_size);

  inited_ = true;
  // AINFO << "init ring_buff min size " << min_size << " size " << size_ << "
  // is_sp " << is_sp_
  //        << " is_sc " << is_sc_;
  return true;
}

// check cons tail
uint32_t RingBuffer::MoveProdHead(uint32_t n, uint32_t* old_head,
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
uint32_t RingBuffer::MoveConsHead(uint32_t n, uint32_t* old_head,
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

uint32_t RingBuffer::DoEnqueue(void* const* obj_table, uint32_t n,
                               uint32_t* free_space, bool is_sp) {
  uint32_t prod_head, prod_next;
  uint32_t free_entries;

  // get range [prod_head, prod_next)
  n = MoveProdHead(n, &prod_head, &prod_next, &free_entries, is_sp);
  if (n == 0) {
    goto end;
  }

  EnqueuePtrs(ring_, prod_head, obj_table, n);
  UpdateTail(&prod_, prod_head, prod_next, is_sp);

end:
  if (free_space != nullptr) {
    *free_space = free_entries - n;
  }
  return n;
}

uint32_t RingBuffer::DoDequeue(void** obj_table, uint32_t n,
                               uint32_t* available, bool is_sc) {
  uint32_t cons_head, cons_next;
  uint32_t entries;

  // get range [cons_head, cons_next)
  n = MoveConsHead(n, &cons_head, &cons_next, &entries, is_sc);
  if (n == 0) {
    goto end;
  }

  DequeuePtrs(ring_, cons_head, obj_table, n);
  UpdateTail(&cons_, cons_head, cons_next, is_sc);

end:
  if (available != nullptr) {
    *available = entries - n;
  }
  return n;
}

int RingBuffer::EnqueuePtrs(void** ring_start, uint32_t prod_head,
                            void* const* obj_table, uint32_t n) {
  uint32_t i;
  const uint32_t size = size_;
  uint32_t idx = prod_head & mask_;
  void** ring = ring_start;

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

int RingBuffer::DequeuePtrs(void** ring_start, uint32_t cons_head,
                            void** obj_table, uint32_t n) {
  uint32_t i;
  uint32_t idx = cons_head & mask_;
  const uint32_t size = size_;
  void** ring = ring_start;
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
