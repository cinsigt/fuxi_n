
#pragma once

#include <stdint.h>
#include <unistd.h>

#include <cstdlib>
#include <thread>

#include "fuxi/base/rw_lock_guard.h"

namespace next {
namespace fuxi {

class PthreadRWLock {
  friend class ReadLockGuard<PthreadRWLock>;
  friend class WriteLockGuard<PthreadRWLock>;

 public:
  explicit PthreadRWLock(bool write_first) {
    pthread_rwlockattr_init(&rwlock_attr_);
    if (write_first) {
      pthread_rwlockattr_setkind_np(
          &rwlock_attr_, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    }
    pthread_rwlockattr_setpshared(
        &rwlock_attr_, PTHREAD_PROCESS_SHARED);  // inter-process sync
    pthread_rwlock_init(&rwlock_, &rwlock_attr_);
  }

  PthreadRWLock() : PthreadRWLock(true) {}

  ~PthreadRWLock() {
    pthread_rwlock_destroy(&rwlock_);
    pthread_rwlockattr_destroy(&rwlock_attr_);
  }
  void ReadLock();
  void WriteLock();

  void ReadUnlock();
  void WriteUnlock();

 private:
  PthreadRWLock(const PthreadRWLock&) = delete;
  PthreadRWLock& operator=(const PthreadRWLock&) = delete;
  pthread_rwlock_t rwlock_;
  pthread_rwlockattr_t rwlock_attr_;
};

inline void PthreadRWLock::ReadLock() { pthread_rwlock_rdlock(&rwlock_); }

inline void PthreadRWLock::WriteLock() { pthread_rwlock_wrlock(&rwlock_); }

inline void PthreadRWLock::ReadUnlock() { pthread_rwlock_unlock(&rwlock_); }

inline void PthreadRWLock::WriteUnlock() { pthread_rwlock_unlock(&rwlock_); }

}  // namespace fuxi
}  // namespace next
