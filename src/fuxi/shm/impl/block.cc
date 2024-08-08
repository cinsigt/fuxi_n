
#include "fuxi/shm/impl/block.h"

#include "fuxi/common/log.h"

namespace next {
namespace fuxi {

Block::Block()
    : is_writing_(false),
      reading_reference_counts_(0),
      msg_size_(0),
      msg_info_size_(0) {}

Block::~Block() {}

bool Block::TryLockForWrite() {
  WriteLockGuard<PthreadRWLock> lock(read_write_mutex_);
  RETURN_VAL_IF2(is_writing_.load(), false);
  RETURN_VAL_IF2(reading_reference_counts_.load() != 0, false);

  // no other own this block, set writing
  // TODO: set pid & ts
  is_writing_.store(true);
  return true;
}

bool Block::TryLockForRead() {
  ReadLockGuard<PthreadRWLock> lock(read_write_mutex_);
  RETURN_VAL_IF2(is_writing_.load(), false);
  reading_reference_counts_.fetch_add(1);
  return true;
}

void Block::ReleaseWriteLock() { is_writing_.store(false); }

void Block::ReleaseReadLock() { reading_reference_counts_.fetch_sub(1); }

}  // namespace fuxi
}  // namespace next
