
#pragma once

#include <memory>
#include <mutex>
#include <string>

// #include "fuxi/mm/pages_mmap.h"

namespace next {
namespace fuxi {

// span is a reserved virtual space
class Span {
 public:
  Span();
  ~Span();

  bool Extend(size_t size, size_t alignment = 4096);

  template <typename T>
  inline T* GetDataPointer() {
    return (T*)((uintptr_t)base_pointer_ + data_offset_);
  }

 private:
  void* base_pointer_{nullptr};
  size_t data_offset_ = 0;

  size_t space_reserved_ = 0;
  size_t space_used_ = 0;
  size_t space_extended_ = 0;
};

}  // namespace fuxi
}  // namespace next
