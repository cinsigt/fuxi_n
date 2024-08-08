
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "fuxi/base/flat_object_queue.h"
#include "fuxi/common/shm_object.h"
#include "fuxi/shm/impl/extent.h"
#include "fuxi/shm/impl/segment_metadata.h"
#include "fuxi/shm/impl/segment_state.h"
#include "fuxi/shm/impl/shm_conf.h"
namespace next {
namespace fuxi {

class Segment;
using SegmentPtr = std::shared_ptr<Segment>;

// 统一从配置中心获取配置
class Segment {
 public:
  explicit Segment(const SegmentOption& opt);
  virtual ~Segment() {}

  bool Initialize(bool create = false);

  uint64_t TotalSegmentSpaceSize() const;
  uint64_t SegmentProtectionSapceSize();

  uint32_t BlockNum() const;
  uint32_t FreeShmObjectNum() const;

  bool GetBlock(const uint8_t* buffer, WritableBlock& block);

  ShmObject* NewObject();

  bool AllocateBlock(ShmObject& object);
  void DeleteObject(ShmObject* object);

  uint32_t CeilingMsgSize() const { return metadata_.CeilingMsgSize(); }

  bool AcquireBlockToWrite(std::size_t msg_size, WritableBlock* writable_block);
  void ReleaseWrittenBlock(const WritableBlock& writable_block);
  bool AcquireBlockToRead(ReadableBlock* readable_block);
  void ReleaseReadBlock(const ReadableBlock& readable_block);

 private:
  bool InitPreOpen();
  bool InitPostOpen();
  bool InitPostCreate();

 protected:
  virtual bool Destroy();

  virtual void Reset() = 0;   // reset and detach
  virtual bool Remove() = 0;  // delete shm
  virtual bool OpenOnly() = 0;
  virtual bool OpenOrCreate() = 0;

  bool inited_{false};
  bool open_flag_{false};
  bool create_flag_{false};
  int32_t shm_id_{0};
  SegmentOption opt_;

  SegmentMetadata metadata_;
  Extent* extent_{nullptr};
  void* segment_base_addr_{nullptr};
  void* head_protection_addr_{nullptr};
  void* managed_shm_header_{nullptr};
  size_t page_size_{0};

 private:
  //
};

}  // namespace fuxi
}  // namespace next
