
#include "fuxi/base/ring_common.h"

namespace {
#if defined(__aarch64__)
static inline void cpu_pause(void) { asm volatile("yield" ::: "memory"); }
#else
#include <emmintrin.h>
// when HT, pause do not consume brother core compute resource
static inline void cpu_pause(void) { _mm_pause(); }
#endif
}  // namespace

namespace next {
namespace fuxi {

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) !!(x)
#define unlikely(x) !!(x)
#endif

void UpdateTail(struct ring_headtail* ht, uint32_t old_val, uint32_t new_val,
                bool single) {
  if (!single) {
    while (unlikely(ht->tail != old_val)) {
      cpu_pause();
    }
  }
  ht->tail.store(new_val, ::std::memory_order_release);
}

}  // namespace fuxi
}  // namespace next
