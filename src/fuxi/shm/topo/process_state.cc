
#include "fuxi/shm/topo/process_state.h"

#include "fuxi/common/time.h"
#include "fuxi/shm/arena/shm_deleter.h"

namespace next {
namespace fuxi {

// class ProcessState
ProcessState::ProcessState() {}
ProcessState::~ProcessState() {}

bool ProcessState::Init(int32_t pid, size_t num, const Buffer& storage) {
  if (inited_) {
    return true;
  }

  reader_event_queue_.Initialize(num, storage);
  pid_ = pid;
  is_online_.store(true);
  ts_ = Time::Now().ToNanosecond();

  inited_ = true;
  return true;
}

int32_t ProcessState::Push(const std::shared_ptr<ShmObject>& object) {
  // try push
  // overflow strategy
  ShmEvent event;
  event.shm_object_addr = (uint64_t)object.get();

  // add reference
  std::cout << "push obj " << object.get() << std::endl;
  ::std::atomic<int32_t>& ref_cnt = object->reference_cnt();
  ref_cnt.fetch_add(1);
  reader_event_queue_.TryPush(std::move(event));
  return 0;
}

std::shared_ptr<ShmObject> ProcessState::Pop() {
  ShmEvent event;
  bool ret = reader_event_queue_.Pop(event);
  if (ret == false) {
    return nullptr;
  }

  ShmObject* obj_ptr = (ShmObject*)event.shm_object_addr;
  ShmObjectDeleter deleter;
  deleter.segment = nullptr;
  deleter.object = (ShmObject*)event.shm_object_addr;
  std::shared_ptr<ShmObject> object(obj_ptr, deleter);
  return object;
}
}  // namespace fuxi
}  // namespace next
