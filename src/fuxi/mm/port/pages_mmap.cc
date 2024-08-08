
#include "fuxi/mm/port/pages_mmap.h"

#include <assert.h>

#include <cstdint>

#include "fuxi/common/log.h"

namespace next {
namespace fuxi {

void* PagesMmap::pages_map(void* addr, size_t size, bool* commit) {
  void* ret;
  assert(size != 0);

  // TODO:
  if (os_overcommits_) {
    *commit = true;
  }

  /*
   * We don't use MAP_FIXED here, because it can cause the *replacement*
   * of existing mappings, and we only want to create new mappings.
   */
  {
    int prot = *commit ? PAGES_PROT_COMMIT : PAGES_PROT_DECOMMIT;
    ret = ::mmap(addr, size, prot, mmap_flags_, pages_fd_tag_, 0);
  }
  assert(ret != nullptr);

  if (ret == MAP_FAILED) {
    ret = nullptr;
  } else if (addr != nullptr && ret != addr) {
    /*
     * We succeeded in mapping memory, but not in the right place.
     */
    pages_unmap(ret, size);
    ret = nullptr;
  }
  assert(ret == nullptr || (addr == nullptr && ret != addr) ||
         (addr != nullptr && ret == addr));
  return (ret);
}

void PagesMmap::pages_unmap(void* addr, size_t size) {
  if (::munmap(addr, size) == -1) {
    // AERROR << "munmap failed " << addr;
  }
}

void* PagesMmap::pages_trim(void* addr, size_t alloc_size, size_t leadsize,
                            size_t size, bool* commit) {
  (void)commit;
  void* ret = (void*)((uintptr_t)addr + leadsize);
  assert(alloc_size >= leadsize + size);

  size_t trailsize = alloc_size - leadsize - size;
  if (leadsize != 0) {
    pages_unmap(addr, leadsize);
  }
  if (trailsize != 0) {
    pages_unmap((void*)((uintptr_t)ret + size), trailsize);
  }
  return (ret);
}

bool PagesMmap::pages_commit_impl(void* addr, size_t size, bool commit) {
  if (os_overcommits_) {
    return (true);
  }

  int prot = commit ? PAGES_PROT_COMMIT : PAGES_PROT_DECOMMIT;
  void* result = ::mmap(addr, size, prot, mmap_flags_ | MAP_FIXED, -1, 0);
  if (result == MAP_FAILED) {
    return (true);
  }

  if (result != addr) {
    /*
     * We succeeded in mapping memory, but not in the right
     * place.
     */
    pages_unmap(result, size);
    return (true);
  }

  return (false);
}

bool PagesMmap::pages_commit(void* addr, size_t size) {
  return (pages_commit_impl(addr, size, true));
}

bool PagesMmap::pages_decommit(void* addr, size_t size) {
  return (pages_commit_impl(addr, size, false));
}

bool PagesMmap::pages_purge(void* addr, size_t size) {
  // MADV_FREE zero false/ MADV_DONTNEED zero true
  // int advice = MADV_FREE;
  // bool zeros = false;

  // zero-fill-on-demand pages
  int advice = MADV_DONTNEED;
  bool zeros = true;
  int err = madvise(addr, size, advice);
  bool unzeroed = (!zeros || err != 0);
  return (unzeroed);
}

bool PagesMmap::pages_huge(void* addr, size_t size) {
  // assert(((void *)((uintptr_t)(a) & ~PAGE_MASK))(addr) == addr);
  // assert(PAGE_CEILING(size) == size);
  return (madvise(addr, size, MADV_HUGEPAGE) != 0);
}

bool PagesMmap::pages_nohuge(void* addr, size_t size) {
  // assert(PAGE_ADDR2BASE(addr) == addr);
  // assert(PAGE_CEILING(size) == size);
  return (madvise(addr, size, MADV_NOHUGEPAGE) != 0);
}

}  // namespace fuxi
}  // namespace next
