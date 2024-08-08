
#pragma once

#include <memory>
#include <mutex>

#include "fuxi/base/pthread_rw_lock.h"
#include "fuxi/base/rte_ring.h"
#include "fuxi/shm/impl/block.h"

namespace next {
namespace fuxi {

struct BlockIndicator {
  uint16_t extent_index{0};
  uint32_t block_index{0};
  Block* block{nullptr};
  uint8_t* buf{nullptr};
};

struct ExtentOption {
  uint64_t offset{0};
  uint64_t base_addr{0};
  uint64_t size{0};
  uint64_t block_num{0};
  uint64_t block_size{0};
};

// make sure multi process app abi and struct layout is same
#pragma pack(push, 8)
struct ExtentLayout {
  uint64_t magic_header{0};
  uint64_t segment_offset{0};  // segment addr as base
  uint64_t space_size{0};
  uint64_t space_used{0};
  uint32_t block_num{0};
  uint32_t block_buffer_size{0};
  uint16_t extent_index{0};
  uint8_t inited{false};

  RteRing<uint16_t> free_block_ids;

  uint32_t block_offset{0};  // &data is base addr
  uint32_t block_buffer_offset{0};
  uint8_t data[0];  // rte_ring_storage & Block & data table

 private:
  friend class SegmentLayout;
  bool Init(const ExtentOption& opt);

 public:
  static uint64_t GetRequestSize(uint64_t block_num, uint64_t block_size) {
    StorageSchema schema;
    RteRing<uint16_t>::GetStorageSchema(schema, block_num);
    return sizeof(ExtentLayout) + block_num * block_size + schema.TotalSize();
  }

  void* Allocate(size_t size);

  Block* BlockTableAddr() { return (Block*)(&(data[0]) + block_offset); }

  uint8_t* BlockBufferBaseAddr() { return &(data[0]) + block_buffer_offset; }
};
#pragma pack(pop)

struct ExtentMetadata {
  ExtentLayout* layout{nullptr};
  Block* block_table{nullptr};

  uint32_t BlockNum() { return layout->block_num; }
  uint32_t BlockBufferSize() { return layout->block_buffer_size; }
  uint32_t ExtentIndex() { return layout->extent_index; }

  Block* GetBlock(uint32_t index) { return &(block_table[index]); }

  bool GetFreeBlockIndex(uint32_t& index) {
    uint16_t id = 0;
    uint32_t ret = layout->free_block_ids.DoDequeue(&id, 1, nullptr);
    if (ret == 0) {
      return false;
    }

    index = id;
    return true;
  }

  bool PutFreeBlockIndex(uint32_t index) {
    // range check
    // refree check
    //
    return true;
  }

  bool TryLockForRead(uint32_t index) { return true; }

  bool ReleaseReadLock(uint32_t index) { return true; }
};

}  // namespace fuxi
}  // namespace next
