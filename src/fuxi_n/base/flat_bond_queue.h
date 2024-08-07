
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>

#include "fuxi/base/flat_object_queue.h"

namespace next {
namespace fuxi {

template <typename T>
class FlatBondQueue {
 public:
  FlatBondQueue();
  ~FlatBondQueue();

 private:
  bool inited_{false};
  size_t object_size_{0};
  size_t queue_depth_{0};

  FlatObjectQueue<T> channel_;
  FlatObjectQueue<T> free_objects_;
};

}  // namespace fuxi
}  // namespace next
