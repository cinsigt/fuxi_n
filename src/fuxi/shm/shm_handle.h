
#pragma once

#include <memory>
#include <string>

#include "fuxi/shm/shm_domain.h"
#include "fuxi/shm/topo/topo_state.h"

namespace next {
namespace fuxi {

struct ReaderHandle {
  int32_t reader_id{-1};
  ReaderSlot* slot{nullptr};
};

struct WriterHandle {
  int32_t writer_id{-1};
  WriterSlot* slot{nullptr};
};

// a shm handle witch attach a named share memory segment
// io type may input or output
class ShmHandle {
 public:
  ShmHandle();
  ~ShmHandle();

  bool Init();

  template <class T>
  std::shared_ptr<T> Get();

  template <class T>
  int Write(const std::shared_ptr<const T>& data);

 private:
  int domain_id_{0};
  std::string channel_name_;

  // dynamic managed resource
  ShmDomain* domain_{nullptr};
  Segment* segment_{nullptr};
  ReaderHandle reader_;
  WriterHandle writer_;

  // HybridPool pool_;  // pool
};

// class ShmHandle impl

}  // namespace fuxi
}  // namespace next
