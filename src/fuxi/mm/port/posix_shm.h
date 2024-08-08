#pragma once

#include <atomic>
#include <string>

#include "fuxi/mm/port/shm_option.h"

namespace next {
namespace fuxi {

class PosixShm {
 public:
  PosixShm();
  ~PosixShm();

  bool Init(const ShmOption& opt);
  void Shutdown();

  static const char* Type() { return "posix"; }

 private:
  bool OpenOrCreate();
  bool OpenOnly();
  bool Remove();
  void Reset();

  bool inited_ = false;

  ShmOption opt_;
  std::string shm_name_;

  void* managed_shm_ = nullptr;
  size_t shm_size_ = 0;
};

}  // namespace fuxi
}  // namespace next
