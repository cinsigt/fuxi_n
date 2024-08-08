
#pragma once

namespace next {
namespace fuxi {

template <typename T>
struct FlatTable {
  size_t reserved_size = 0;
  size_t allocated_size = 0;

  T table[0];
};

}  // namespace fuxi
}  // namespace next
