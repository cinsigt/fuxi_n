
#include "fuxi/shm/impl/extent.h"

#include "fuxi/common/log.h"
#include "fuxi/common/time.h"
#include "fuxi/common/util.h"

namespace next {
namespace fuxi {

Extent::Extent()
    : init_(false),
      block_buf_lock_(),
      block_buf_addrs_(),
      page_size_(sysconf(_SC_PAGESIZE)) {}

Extent::~Extent() {}

bool Extent::Initialize(void* extent_base_addr) {
  if (init_) {
    return true;
  }

  metadata_.layout = reinterpret_cast<ExtentLayout*>(extent_base_addr);
  metadata_.block_table = metadata_.layout->BlockTableAddr();

  // 2 create block buffer table
  uint8_t* tmp_addr = metadata_.layout->BlockBufferBaseAddr();
  uint32_t i = 0;
  std::cout << "tmp addr " << (void*)tmp_addr << " buffsize "
            << metadata_.BlockBufferSize() << " blocknum "
            << metadata_.BlockNum() << std::endl;
  block_buf_addrs_.resize(metadata_.BlockNum());
  for (; i < metadata_.BlockNum(); ++i) {
    uint8_t* addr =
        static_cast<uint8_t*>(tmp_addr) + i * metadata_.BlockBufferSize();

    std::lock_guard<std::mutex> _g(block_buf_lock_);
    block_buf_addrs_[i] = addr;
  }

  std::cout << "extent block num " << metadata_.BlockNum() << " block size "
            << metadata_.BlockBufferSize() << " extent addr "
            << extent_base_addr << std::endl;
  init_ = true;
  return true;
}

bool Extent::AcquireBlockToWrite(std::size_t msg_size,
                                 WritableBlock* writable_block) {
  RETURN_VAL_IF_NULL2(writable_block, false);

  uint32_t index = 0;
  if (false == metadata_.GetFreeBlockIndex(index)) {
    std::cout << "index " << index << std::endl;
    return false;
  }
  std::cout << "get index " << index << " buf " << block_buf_addrs_[index]
            << std::endl;
  writable_block->extent_index = metadata_.ExtentIndex();
  writable_block->block_index = index;
  writable_block->block = metadata_.GetBlock(index);
  writable_block->buf = block_buf_addrs_[index];
  return true;
}

void Extent::ReleaseWrittenBlock(const WritableBlock& writable_block) {
  metadata_.PutFreeBlockIndex(writable_block.block_index);
}

// acquire a readable block
bool Extent::AcquireBlockToRead(ReadableBlock* readable_block) {
  RETURN_VAL_IF_NULL2(readable_block, false);

  auto index = readable_block->block_index;
  RETURN_VAL_IF2(!metadata_.TryLockForRead(index), false);

  readable_block->block = metadata_.GetBlock(index);
  readable_block->buf = block_buf_addrs_[index];
  return true;
}

void Extent::ReleaseReadBlock(const ReadableBlock& readable_block) {
  auto index = readable_block.block_index;
  metadata_.ReleaseReadLock(index);
}

uint32_t Extent::GetBlockNum() { return metadata_.BlockNum(); }

}  // namespace fuxi
}  // namespace next
