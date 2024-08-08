
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>

#include "fuxi/base/pthread_rw_lock.h"

namespace next {
namespace fuxi {

using ::next::fuxi::PthreadRWLock;
using ::next::fuxi::ReadLockGuard;
using ::next::fuxi::WriteLockGuard;

class Block {
  // friend class Segment;
  friend class Extent;

 public:
  Block();
  virtual ~Block();

  uint64_t MsgSize() const { return msg_size_; }
  void SetMsgSize(uint64_t msg_size) { msg_size_ = msg_size; }

  uint64_t MsgInfoSize() const { return msg_info_size_; }
  void SetMsgInfoSize(uint64_t msg_info_size) {
    msg_info_size_ = msg_info_size;
  }

  void SetWUser(bool has_w_user);
  uint32_t AddRUser();
  uint32_t ReduceRUser();
  uint32_t GetRUser() const;

 private:
  bool TryLockForWrite();
  bool TryLockForRead();
  void ReleaseWriteLock();
  void ReleaseReadLock();

  // inter process shared data
  // TODO: minimize field
  std::atomic<bool> is_writing_{0};
  std::atomic<uint32_t> reading_reference_counts_{0};

  PthreadRWLock read_write_mutex_;

  uint64_t msg_size_{0};
  uint64_t msg_info_size_{0};
};

}  // namespace fuxi
}  // namespace next
