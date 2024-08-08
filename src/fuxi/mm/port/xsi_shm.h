
#pragma once

#include <stdint.h>
#include <sys/types.h>

#include <atomic>

#include "fuxi/mm/port/shm_option.h"

namespace next {
namespace fuxi {

class XsiShm {
 public:
  XsiShm();
  ~XsiShm();

  bool Init(const ShmOption& opt);
  void Shutdown();

  bool IsCreated() const { return is_created_; }

  void* BaseAddr() { return managed_shm_; }

  size_t SpaceSize() const { return shm_size_; }

  static const char* Type() { return "xsi"; }

 private:
  bool OpenOrCreate();
  bool OpenOnly();
  bool Remove();
  void Reset();

  bool inited_ = false;
  bool is_created_ = false;

  ShmOption opt_;
  std::string path_;
  key_t key_ = 0;

  void* managed_shm_ = nullptr;
  size_t shm_size_ = 0;
};

}  // namespace fuxi
}  // namespace next
