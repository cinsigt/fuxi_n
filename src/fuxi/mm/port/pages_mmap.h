
#include <absl/numeric/bits.h>
#include <sys/mman.h>

namespace next {
namespace fuxi {

class PagesMmap {
 public:
  PagesMmap() = default;

  inline void set_fd(int fd) { pages_fd_tag_ = fd; }

  inline void set_page_size(size_t page_size) {
    page_size_ = ::absl::bit_ceil(page_size);
    page_mask_ = page_size_ - 1;
  }

  // commit or not commit
  void* pages_map(void* addr, size_t size, bool* commit);
  void* pages_trim(void* addr, size_t alloc_size, size_t leadsize, size_t size,
                   bool* commit);
  void pages_unmap(void* addr, size_t size);

  // 返还虚拟内存
  bool pages_commit(void* addr, size_t size);
  bool pages_decommit(void* addr, size_t size);
  bool pages_commit_impl(void* addr, size_t size, bool commit);

  // 清零
  bool pages_purge(void* addr, size_t size);

  bool pages_huge(void* addr, size_t size);
  bool pages_nohuge(void* addr, size_t size);

  static constexpr int DEFAULT_PAGE_PROT = (PROT_READ | PROT_WRITE);
  static constexpr int DEFAULT_MMAP_FLAGS = (MAP_PRIVATE | MAP_ANONYMOUS);
  static constexpr int DEFAULT_PAGES_FD_TAG = -1;
  static constexpr int PAGES_PROT_COMMIT = (PROT_READ | PROT_WRITE);
  static constexpr int PAGES_PROT_DECOMMIT = (PROT_NONE);

 private:
  size_t page_size_ = 4096;  // PAGE_SIZE;
  size_t page_mask_ = 4096 - 1;
  bool os_overcommits_ = true;  // always commit
  int prot_{DEFAULT_PAGE_PROT};
  int mmap_flags_{DEFAULT_MMAP_FLAGS};
  int pages_fd_tag_{DEFAULT_PAGES_FD_TAG};
};

}  // namespace fuxi
}  // namespace next
