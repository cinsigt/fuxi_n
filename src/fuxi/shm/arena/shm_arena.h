
#pragma once

#include <memory>

#include "fuxi/common/shm_object.h"
#include "fuxi/mm/allocator.h"
#include "fuxi/mm/page_allocator.h"
#include "fuxi/shm/arena/shm_deleter.h"
#include "fuxi/shm/impl/segment.h"

namespace next {
namespace fuxi {

// 1. pmr allocator --> memory_resource --> blocker allocator
// 2. shm arena support allocator prototype use allocator::construct
class ShmArena : public PageAllocator {
 public:
  ShmArena();
  ShmArena(Segment* segment, size_t block_size);
  ~ShmArena();

  bool Init(Segment* segment, size_t block_size);

  virtual size_t page_size() const noexcept override;

  using PageAllocator::allocate;
  virtual void allocate(void** blocks, size_t num) noexcept override;
  using PageAllocator::deallocate;
  virtual void deallocate(void** blocks, size_t num) noexcept override;

  // new pb message
  template <typename T>
  std::shared_ptr<T> NewObject() {
    // avoid multiple make
    if (is_owner_ == false) {
      return nullptr;
    }
    if (block_index_table_.size() > ShmObject::MAX_BLOCK_NUM) {
      return nullptr;
    }

    // to protobuf arena
    // google::protobuf::Arena & arena = resource_;
    // T* ptr = google::protobuf::Arena::CreateMessage<T>(&arena);

    auto allocator = SwissAllocator<T>(resource_);
    T* ptr = allocator.template new_object<T>();
    return MakeSharedAndMove(ptr);
  }

  // move resource owner from arena to shared_ptr<T>
  // resource release when shared_ptr<> reference is zero,
  // then call ShmObjectDeleter
  template <typename T>
  std::shared_ptr<T> MakeSharedAndMove(T* ptr) {
    // avoid multiple make
    if (is_owner_ == false) {
      return nullptr;
    }
    if (block_index_table_.size() > ShmObject::MAX_BLOCK_NUM) {
      return nullptr;
    }

    ShmObjectDeleter deleter;
    deleter.segment = segment_;
    deleter.object = object_;
    deleter.object->data_addr = (uint64_t)ptr;

    std::shared_ptr<T> shared_ptr(ptr, deleter);
    if (shared_ptr) {
      is_owner_ = false;
    }
    return shared_ptr;
  }

 private:
  bool inited_{false};
  bool is_owner_{true};  // arena resource release strategy
  size_t block_size_{4096};
  std::vector<uint32_t> block_index_table_;

  // point to shm addr
  Segment* segment_{nullptr};
  ShmObject* object_{nullptr};  // underlay shm object

  SwissMemoryResource resource_;
};

}  // namespace fuxi
}  // namespace next
