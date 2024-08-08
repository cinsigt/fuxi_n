
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "fuxi/base/flat_object_queue.h"
#include "fuxi/common/shm_object.h"
#include "fuxi/shm/impl/extent.h"
#include "fuxi/shm/impl/segment_state.h"
#include "fuxi/shm/impl/shm_conf.h"

namespace next {
namespace fuxi {

struct SegmentOption {
  std::string name;
  uint64_t channel_id{0};
  int32_t domain_id{0};
  void* base_addr{nullptr};
  uint64_t space_size{0};
  size_t block_num{0};
  size_t block_size{0};
};

// make sure all field is flat
#pragma pack(push, 8)
struct SegmentLayout {
  uint64_t magic_header{0};
  uint32_t block_size{0};
  uint32_t block_num{0};
  uint16_t header_size{0};  // multi ptocess check
  uint16_t init{0};
  uint32_t epoch{0};  // global epoch
  uint64_t space_size{0};
  uint64_t space_used{0};

  PthreadRWLock rw_mutex;

  SegmentState state;
  FlatObjectQueue<ShmObject> object_pool;
  uint64_t extent_offset{0};  // base is &data[0]
  uint8_t data[0];            // shm object pool data, extent

  // function
  void Init(const SegmentOption& opt);
  ExtentLayout* GetExtentLayout();
  std::shared_ptr<Extent> MakeExtent();
  void* Allocate(size_t size, size_t alignment = 128);
};
#pragma pack(pop)

struct SegmentMetadata {
  SegmentLayout* layout{nullptr};
  std::shared_ptr<Extent> extent{nullptr};

  uint32_t BlockNum() const { return layout->block_num; }

  uint32_t FreeShmObjectNum() const { return layout->object_pool.FreeNum(); }

  inline ShmObject* GetObject() { return layout->object_pool.Get(); }

  inline int PutObject(ShmObject* object) {
    return layout->object_pool.Put(object);
  }

  SegmentState* GetState() { return &(layout->state); }

  uint32_t CeilingMsgSize() const { return layout->state.CeilingMsgSize(); }

  bool IsInit() const { return layout->state.IsInit(); }
  void Destroy() {}
};

}  // namespace fuxi
}  // namespace next
