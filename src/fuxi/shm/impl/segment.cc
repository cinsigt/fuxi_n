
#include "fuxi/shm/impl/segment.h"

#include "fuxi/common/log.h"
#include "fuxi/common/util.h"

namespace next {
namespace fuxi {

Segment::Segment(const SegmentOption& opt)
    : inited_(false),
      open_flag_(false),
      opt_(opt),
      page_size_(sysconf(_SC_PAGESIZE)) {
  opt_.space_size += SegmentProtectionSapceSize();
  std::cout << "segment spread from " << opt.space_size << " to "
            << opt_.space_size << std::endl;
}

// segment init
bool Segment::Initialize(bool create) {
  if (inited_ == true) {
    return true;
  }

  InitPreOpen();

  bool ret = false;
  if (create) {
    ret = OpenOrCreate();
  } else {
    ret = OpenOnly();
  }

  if (false == ret) {
    // open failed
    std::cout << "open shm name path failed" << std::endl;
    return false;
  }

  if (create_flag_) {
    ret = InitPostCreate();
  } else {
    ret = InitPostOpen();
  }
  if (false == ret) {
    // open failed
    std::cout << "init post failed" << std::endl;
    return false;
  }

  inited_ = true;
  return true;
}

bool Segment::InitPostCreate() {
  // explicit init post
  // only one process thread reach here
  SegmentLayout* layout = nullptr;
  layout = new (managed_shm_header_) SegmentLayout();
  layout->Init(opt_);

  metadata_.layout = layout;
  metadata_.extent = layout->MakeExtent();
  extent_ = metadata_.extent.get();

  // Init state
  SegmentState* state = metadata_.GetState();
  state->IncreaseReferenceCount();
  state->SetShmId(shm_id_);
  state->Init(true);
  return true;
}

bool Segment::InitPostOpen() {
  SegmentLayout* layout = reinterpret_cast<SegmentLayout*>(managed_shm_header_);
  metadata_.layout = layout;

  // wait create finished
  size_t retry_times = 0;
  while (metadata_.IsInit() == false) {
    std::this_thread::sleep_for(std::chrono::microseconds(1000));  // 1ms
    retry_times++;
    if (++retry_times > 2000) {  // 2s
      // timeout return
      return false;
    }
  }

  // TODO: version && domain check
  //
  metadata_.extent = layout->MakeExtent();
  extent_ = metadata_.extent.get();
  SegmentState* state = metadata_.GetState();
  state->IncreaseReferenceCount();
  // add reference count
  return true;
}

bool Segment::InitPreOpen() {
  if (page_size_ == 0) {
    page_size_ = sysconf(_SC_PAGESIZE);
  }
  if (opt_.space_size < SegmentProtectionSapceSize()) {
    return false;
  }

  uint32_t block_num =
      (opt_.space_size - SegmentProtectionSapceSize()) / opt_.block_size;
  if ((block_num < opt_.block_num) || (opt_.block_num == 0)) {
    std::cout << "block num change from " << opt_.block_num << " to "
              << block_num << " space_size " << opt_.space_size << std::endl;
    opt_.block_num = block_num;
  }
  segment_base_addr_ = opt_.base_addr;
  return true;
}

uint32_t Segment::BlockNum() const { return metadata_.BlockNum(); }

uint32_t Segment::FreeShmObjectNum() const {
  return metadata_.FreeShmObjectNum();
}

uint64_t Segment::TotalSegmentSpaceSize() const { return opt_.space_size; }

uint64_t Segment::SegmentProtectionSapceSize() {
  if (page_size_ == 0) {
    page_size_ = sysconf(_SC_PAGESIZE);
  }
  return page_size_ * 2;
}

ShmObject* Segment::NewObject() {
  if (inited_ == false) {
    // TODO: lock guard
    bool ret = Initialize(true);
    if (ret == false) {
      // log open shm error
      std::cout << "init failed" << std::endl;
      return nullptr;
    }
  }

  return metadata_.GetObject();
}

bool Segment::GetBlock(const uint8_t* buffer, WritableBlock& block) {
  // TODO:
  return true;
}

bool Segment::AllocateBlock(ShmObject& object) {
  // TODO
  return true;
}

void Segment::DeleteObject(ShmObject* object) {
  // TODO
  return;
}

bool Segment::AcquireBlockToWrite(std::size_t msg_size,
                                  WritableBlock* writable_block) {
  RETURN_VAL_IF_NULL2(writable_block, false);

  if (inited_ == false) {
    // TODO: lock guard
    bool ret = Initialize(true);
    if (ret == false) {
      // log open shm error
      std::cout << "init failed" << std::endl;
      return false;
    }
  }

  if (msg_size > CeilingMsgSize()) {
    std::cout << "msg_size: " << msg_size
              << " larger than current shm_buffer_size: " << CeilingMsgSize()
              << std::endl;
    return false;
  }

  return extent_->AcquireBlockToWrite(msg_size, writable_block);
}

void Segment::ReleaseWrittenBlock(const WritableBlock& writable_block) {
  return extent_->ReleaseWrittenBlock(writable_block);
}

bool Segment::AcquireBlockToRead(ReadableBlock* readable_block) {
  if (inited_ == false) {
    // TODO: lock guard
    bool ret = Initialize(false);
    if (ret == false) {
      // log open shm error
      return false;
    }
  }

  return extent_->AcquireBlockToRead(readable_block);
}

void Segment::ReleaseReadBlock(const ReadableBlock& readable_block) {
  return extent_->ReleaseReadBlock(readable_block);
}

bool Segment::Destroy() {
  RETURN_VAL_IF2(!inited_, true);
  inited_ = false;

  // TODO:
  SegmentState* state = metadata_.GetState();
  int32_t reference_count = state->DecreaseReferenceCount();
  if (reference_count == 0) {
    std::cout << "call remove" << std::endl;
    Remove();
  }

  metadata_.Destroy();
  return true;
}

}  // namespace fuxi
}  // namespace next
