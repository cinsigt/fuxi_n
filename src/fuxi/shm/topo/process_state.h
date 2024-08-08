
#pragma once

#include "fuxi/base/flat_object_queue.h"
#include "fuxi/base/flat_signal_queue.h"
#include "fuxi/base/pthread_rw_lock.h"
#include "fuxi/common/shm_object.h"
#include "fuxi/shm/topo/shm_meta.h"

namespace next {
namespace fuxi {

// class ProcessState
// output channel:
// writer_id --> writer slot --> reader ids --> reader process state id
// --> reader event queue push
// input channel:
// spin --> dispatch event --> event process
class ProcessState {
 public:
  ProcessState();
  ~ProcessState();

  bool Init(int32_t pid, size_t num, const Buffer& storage);

  inline void SetPidOnline(int32_t pid, bool online) {
    pid_ = pid;
    is_online_.store(online);
  }
  int32_t GetPid() const { return pid_; }

  // share data to process level readers
  int32_t Push(const std::shared_ptr<ShmObject>& object);

  std::shared_ptr<ShmObject> Pop();

 public:
  static void GetStorageSchema(StorageSchema& schema, size_t num) {
    FlatSignalQueue<ShmEvent>::GetStorageSchema(schema, num);
  }

  bool SetPlacementStorage(const PlacementStorage& storage) {
    return reader_event_queue_.SetPlacementStorage(storage);
  }

 private:
  bool inited_{false};
  std::atomic<bool> is_online_{false};
  int32_t pid_{0};
  uint64_t ts_{0};

  // per process level reader event queue
  // need placement storage
  FlatSignalQueue<ShmEvent> reader_event_queue_;
};

}  // namespace fuxi
}  // namespace next
