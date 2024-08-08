
#include "fuxi/shm/arena/shm_arena.h"

namespace next {
namespace fuxi {

ShmArena::ShmArena() {}

ShmArena::ShmArena(Segment* segment, size_t block_size) {
  Init(segment, block_size);
}

ShmArena::~ShmArena() {
  if (false == is_owner_) {
    return;
  }

  if (segment_ && object_) {
    segment_->DeleteObject(object_);
    object_ = nullptr;
    segment_ = nullptr;
  }
}

bool ShmArena::Init(Segment* segment, size_t block_size) {
  if (inited_) {
    return true;
  }

  block_size_ = block_size;
  block_index_table_.reserve(ShmObject::MAX_BLOCK_NUM);
  segment_ = segment;
  object_ = segment->NewObject();
  // check shm_object_->ref_cnt == 1
  resource_.set_page_allocator(*this);
  inited_ = true;
  return true;
}

size_t ShmArena::page_size() const noexcept { return block_size_; }

void ShmArena::allocate(void** blocks, size_t num) noexcept {
  WritableBlock block;
  for (size_t i = 0; i < num; ++i) {
    bool ret = segment_->AcquireBlockToWrite(block_size_, &block);
    if (ret == false) {
      return;
    }

    // record
    block_index_table_.push_back(block.block_index);
    if (object_->block_num < ShmObject::MAX_BLOCK_NUM) {
      object_->block_index[object_->block_num++] = block.block_index;
    }
    blocks[i] = block.buf;
  }
}

void ShmArena::deallocate(void** blocks, size_t num) noexcept {
  // do nothing
  return;
  WritableBlock block;
  for (size_t i = 0; i < num; ++i) {
    bool ret =
        segment_->GetBlock(reinterpret_cast<const uint8_t*>(blocks[i]), block);
    if (ret == false) {
      continue;
    }

    // erase recorded block index
    segment_->ReleaseWrittenBlock(block);
  }
}

}  // namespace fuxi
}  // namespace next
