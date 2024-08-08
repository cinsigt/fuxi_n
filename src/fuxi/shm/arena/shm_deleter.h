
#pragma once

#include <memory>

#include "fuxi/shm/impl/segment.h"

namespace next {
namespace fuxi {

// demo:
// construct message:
// ShmObjectDeleter deleter;
// fill deleter
//
// std::shared_ptr<T> msg(ptr, deleter);
//
// get shm object from shared_ptr<T>
// ShmObjectDeleter *deleter = std::get_deleter<ShmObjectDeleter>(msg);
struct ShmObjectDeleter {
  // deleter function
  void operator()(void const* p) {
    if (nullptr == p) {
      return;
    }
    if ((object == nullptr) || (segment == nullptr)) {
      return;
    }
    if ((uint64_t)p != object->data_addr) {
      return;
    }

    // sub reference cnt and check last reference cnt
    ::std::atomic<int32_t>& ref_cnt = object->reference_cnt();
    int32_t current = ref_cnt.fetch_sub(1) - 1;
    if (current == 0) {
      // reference count is 0, delete object
      segment->DeleteObject(object);
      segment = nullptr;
      object = nullptr;
    }
    return;
  }

  // static interface
  template <typename T>
  static std::shared_ptr<ShmObject> MakeShmObject(
      const std::shared_ptr<T>& msg) {
    ShmObjectDeleter* deleter = std::get_deleter<ShmObjectDeleter>(msg);
    if (deleter == nullptr) {
      return nullptr;
    }

    // TODO: do more check if should transform to ipc ShmObject
    ShmObject* shm_object_ptr = deleter->object;
    ::std::atomic<int32_t>& ref_cnt = shm_object_ptr->reference_cnt();
    ref_cnt.fetch_add(1);

    ShmObjectDeleter self_deleter;
    self_deleter.segment = deleter->segment;
    self_deleter.object = shm_object_ptr;
    std::shared_ptr<ShmObject> shm_object(shm_object_ptr, self_deleter);
    return shm_object;
  }

  template <typename T>
  static std::shared_ptr<const T> ParseFromShmObject(
      const std::shared_ptr<ShmObject>& obj) {
    ShmObjectDeleter deleter;
    obj->IncreaseReferenceCount();
    deleter.object = obj.get();  // add shm object reference
    // deleter.segment = obj->segment;
    T *ptr = (T*)(obj->data_addr);
    std::shared_ptr<T> msg(ptr, deleter);
    return msg;
  }

  // manage data
  Segment* segment{nullptr};
  ShmObject* object{nullptr};
};

}  // namespace fuxi
}  // namespace next
