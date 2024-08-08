
#pragma once

#include <atomic>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include "fuxi/base/pthread_rw_lock.h"

namespace next {
namespace fuxi {

using ::next::fuxi::PthreadRWLock;
using ::next::fuxi::WriteLockGuard;

class SegmentState {
 public:
  explicit SegmentState();
  virtual ~SegmentState();

  bool SetSegmentSpan(uint64_t base_addr, uint64_t size);

  void SetBlockSize(uint32_t msg_size) { ceiling_msg_size_.store(msg_size); }

  bool IsInit() const { return inited_.load(); };

  void Init(bool value) { inited_.store(value); }

  void SetShmId(int shm_id) { shm_id_.store(shm_id); }

  uint64_t GetSize() const;

  uint32_t CeilingMsgSize() const {
    return ceiling_msg_size_.load(std::memory_order_relaxed);
  }

  uint32_t FetchAddSeq(uint32_t diff) { return seq_.fetch_add(diff); }

  uint32_t Seq() { return seq_.load(); }

  int32_t IncreaseReferenceCount() {
    return (reference_count_.fetch_add(1) + 1);
  }

  int32_t DecreaseReferenceCount() {
    return (reference_count_.fetch_sub(1) - 1);
#if 0
    uint32_t current_reference_count = reference_count_.load();
    do {
      if (current_reference_count == 0) {
        return;
      }
    } while (!reference_count_.compare_exchange_strong(
        current_reference_count, current_reference_count - 1));
#endif
  }

  uint32_t ReferenceCount() { return reference_count_.load(); }

 private:
  std::atomic<bool> inited_{false};
  std::atomic<int32_t> shm_id_{0};
  std::atomic<int32_t> reference_count_{0};
  std::atomic<uint32_t> seq_{0};
  std::atomic<uint32_t> ceiling_msg_size_{0};
  std::atomic<uint64_t> segment_base_addr_{0};  // check
  std::atomic<uint64_t> segment_size_{0};
};

}  // namespace fuxi
}  // namespace next
