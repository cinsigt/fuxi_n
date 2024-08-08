

#include "fuxi/shm/impl/segment_metadata.h"

#include "fuxi/shm/impl/extent_metadata.h"
#include "fuxi/shm/topo/shm_meta.h"

namespace next {
namespace fuxi {

void SegmentLayout::Init(const SegmentOption& opt) {
  // use mutex
  WriteLockGuard<PthreadRWLock> lock(rw_mutex);
  if (init) {
    return;
  }

  uint8_t* segment_base = reinterpret_cast<uint8_t*>(this);
  header_size = sizeof(*this);
  magic_header = ShmMeta::SEGMENT_MAGIC_HEADER;
  block_num = opt.block_num;
  block_size = opt.block_size;
  space_size = opt.space_size;

  // init segment state
  state.SetSegmentSpan((uint64_t)segment_base, space_size);
  state.SetBlockSize(block_size);

  // init free object pool
  StorageSchema schema;
  object_pool.GetStorageSchema(schema, block_num);

  PlacementStorage storage;
  storage.resize(schema.FieldSize());
  for (size_t i = 0; i < schema.FieldSize(); ++i) {
    storage.field_buffers[i].ptr =
        Allocate(schema.fields[i].request_space_size);
  }
  object_pool.Initialize(block_num, storage);

  // add a guard

  // init extent
  ::next::fuxi::ExtentOption extent_opt;
  // extent_opt.offset = space_used;
  extent_opt.block_num = block_num;
  extent_opt.block_size = block_size;
  extent_opt.size = ExtentLayout::GetRequestSize(block_num, block_size);
  extent_opt.base_addr = (uint64_t)Allocate(extent_opt.size);
  extent_offset = extent_opt.base_addr - (uint64_t)(&data[0]);
  extent_opt.offset = extent_offset;

  std::cout << "init segment layout block num " << block_num << " offset "
            << extent_offset << std::endl;

  ExtentLayout* extent_layout =
      new ((void*)extent_opt.base_addr) ExtentLayout();
  extent_layout->Init(extent_opt);

  // init done
  init = true;
  return;
}

void* SegmentLayout::Allocate(size_t size, size_t alignment) {
  alignment = ::absl::bit_ceil(alignment);
  size_t bytes = (size + alignment - 1) & static_cast<size_t>(-alignment);

  // check remain
  void* ptr = &(data[0]) + space_used;
  space_used += bytes;
  return ptr;
}

ExtentLayout* SegmentLayout::GetExtentLayout() {
  ExtentLayout* extent = nullptr;
  extent = reinterpret_cast<ExtentLayout*>(&(data[0]) + extent_offset);
  return extent;
}

std::shared_ptr<Extent> SegmentLayout::MakeExtent() {
  std::shared_ptr<Extent> extent = std::make_shared<Extent>();
  extent->Initialize(GetExtentLayout());
  return extent;
}

}  // namespace fuxi
}  // namespace next
