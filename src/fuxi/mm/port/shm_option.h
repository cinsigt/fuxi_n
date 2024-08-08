
#pragma once

#include <stdint.h>

#include <string>

namespace next {
namespace fuxi {

struct ShmOption {
  int32_t domain_id{0};
  int32_t name_id{0};
  std::string path;
  std::string name;
  uint64_t size{0};
  void* addr{nullptr};
};

}  // namespace fuxi
}  // namespace next
