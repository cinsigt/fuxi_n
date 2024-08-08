
#include "fuxi/shm/impl/extent_metadata.h"

#include "fuxi/shm/topo/shm_meta.h"

namespace next {
namespace fuxi {

bool ExtentLayout::Init(const ExtentOption& opt) {
  if (inited) {
    std::cout << "already inited" << std::endl;
    return true;
  }
  if (opt.base_addr == 0) {
    std::cout << "base addr is null" << std::endl;
    return false;
  }

  magic_header = ShmMeta::EXTENT_MAGIC_HEADER;
  segment_offset = opt.offset;
  space_size = opt.size;
  block_num = opt.block_num;
  block_buffer_size = opt.block_size;

  // free block ids
  StorageSchema schema;
  free_block_ids.GetStorageSchema(schema, block_num);
  PlacementStorage storage;
  storage.resize(schema.FieldSize());
  for (size_t i = 0; i < schema.FieldSize(); ++i) {
    storage.field_buffers[i].ptr =
        Allocate(schema.fields[i].request_space_size);
  }
  free_block_ids.Init(block_num, false, false, storage);

  uint16_t idx = 0;
  for (; idx < (uint16_t)block_num; ++idx) {
    uint32_t ret = free_block_ids.DoEnqueue(&idx, 1, nullptr);
    if (ret != 1) {
      // TODO: error
      std::cout << "enqueue free block id failed" << std::endl;
    } else {
      // std::cout << "enqueue id " << idx << " free_block_ids " <<
      // &free_block_ids
      //          << std::endl;
    }
  }

  std::cout << "init extent layout block num " << block_num
            << " extent layout addr " << this << " base addr " << opt.base_addr
            << std::endl;

  // init BlockTable
  Block* blocks = reinterpret_cast<Block*>(Allocate(sizeof(Block) * block_num));
  blocks = new ((void*)blocks) Block[block_num];
  block_offset = reinterpret_cast<uint8_t*>(blocks) - &data[0];

  uint8_t* block_buffer_base =
      reinterpret_cast<uint8_t*>(Allocate(block_buffer_size * block_num));
  block_buffer_offset = block_buffer_base - &data[0];
  uint32_t i = 0;
  for (; i < block_num; ++i) {
    uint8_t* addr = new (block_buffer_base + i * block_buffer_size)
        uint8_t[block_buffer_size];
  }

  inited = true;
  return true;
}

void* ExtentLayout::Allocate(size_t size) {
  // check remain
  void* ptr = &(data[0]) + space_used;
  space_used += size;
  return ptr;
}

}  // namespace fuxi
}  // namespace next
