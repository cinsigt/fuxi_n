

#include "fuxi/shm/impl/span.h"

#include <sys/mman.h>

namespace next {
namespace fuxi {

bool Span::Extend(size_t size, size_t alignment) {
  // check alignment
  // aligned page size
  size_t aligned = (size + alignment - 1) & static_cast<size_t>(-alignment);

  // mprotect:
  void* new_addr = (void*)((uintptr_t)base_pointer_ + space_extended_);
  int prot = (PROT_READ | PROT_WRITE);
  int ret = mprotect(new_addr, aligned, prot);
  if (ret != 0) {
    return false;
  }

  space_extended_ += aligned;
  return true;
}

}  // namespace fuxi
}  // namespace next
