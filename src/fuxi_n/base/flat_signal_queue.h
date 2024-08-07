
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>

#include "fuxi/base/bounded_queue.h"
#include "fuxi/base/ring_common.h"
#include "fuxi/base/storage_schema.h"

namespace next {
namespace fuxi {

template <typename T>
class FlatSignalQueue {
 public:
  FlatSignalQueue();
  ~FlatSignalQueue();

  bool Initialize(size_t num, const Buffer& placement_storage);

  // non blocking if queue is full
  bool TryPush(T&& value);

  // blocking if queue is full
  bool Push(T&& value);

  // non blocking pop
  bool TryPop(T& value);

  // blocking if queue is empty
  bool Pop(T& value);

 public:
  static void GetStorageSchema(StorageSchema& schema, size_t num) {
    schema.fields.resize(1);
    schema.fields[0].field_id = 0;
    schema.fields[0].repeated_size = num;

    schema.fields[0].request_space_size =
        ConcurrentBoundedQueue<T>::reserve_buffer_size(num);
  }

  bool SetPlacementStorage(const PlacementStorage& storage) { return true; }

 private:
  // initial state is an empty queue
  bool inited_{false};
  size_t queue_capacity_{0};
  std::atomic<int32_t> available_size_{0};  // tracking queue size
  ConcurrentBoundedQueue<T> ev_queue_;
};

// class impl
template <typename T>
FlatSignalQueue<T>::FlatSignalQueue() {}

template <typename T>
FlatSignalQueue<T>::~FlatSignalQueue() {}

template <typename T>
bool FlatSignalQueue<T>::Initialize(size_t num,
                                    const Buffer& placement_storage) {
  if (inited_) {
    // already inited
    return true;
  }

  // init ev_queue
  ev_queue_.reserve_and_clear(placement_storage.ptr, num);
  queue_capacity_ = num;

  inited_ = true;
  return true;
}

template <typename T>
bool FlatSignalQueue<T>::TryPush(T&& value) {
  int32_t current_size =
      available_size_.fetch_add(1, std::memory_order_seq_cst);
  if (current_size >= queue_capacity_) {
    // overflow
    available_size_.fetch_sub(1, std::memory_order_seq_cst);
    return false;
  }

  bool ret = false;
  do {
    // concurrent and wakep
    ret = ev_queue_.template try_push<true, true>(std::move(value));
  } while (ret == false);

  return true;
}

template <typename T>
bool FlatSignalQueue<T>::Push(T&& value) {
  // concurrent, futex wait, futex wakeup
  ev_queue_.template push<true, true, true>(std::move(value));
  return true;
}

template <typename T>
bool FlatSignalQueue<T>::TryPop(T& value) {
  int32_t current_size =
      available_size_.fetch_sub(1, std::memory_order_seq_cst);
  if (current_size < 1) {
    available_size_.fetch_add(1, std::memory_order_seq_cst);
    return false;
  }

  auto callback =
      [&value](typename ::next::fuxi::ConcurrentBoundedQueue<T>::Iterator begin,
               typename ::next::fuxi::ConcurrentBoundedQueue<T>::Iterator end) {
        (void)end;
        value = *begin;
      };
  size_t num = 0;
  do {
    num = ev_queue_.template try_pop_n<true, false>(std::move(callback), 1);
  } while (num == 0);

  return true;
}

template <typename T>
bool FlatSignalQueue<T>::Pop(T& value) {
  // concurrent, futex wait, futex wakeup
  ev_queue_.template pop<true, true, true>(value);
  return true;
}

}  // namespace fuxi
}  // namespace next
