#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <atomic>

namespace next {
namespace fuxi {

struct ring_headtail {
  alignas(128) std::atomic<uint32_t> head{0};
  alignas(128) std::atomic<uint32_t> tail{0};
  // uint32_t single = true;
};

void UpdateTail(struct ring_headtail* ht, uint32_t old_val, uint32_t new_val,
                bool single);

}  // namespace fuxi
}  // namespace next
