#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>

#include "fuxi/base/rte_ring.h"
#include "fuxi/base/storage_schema.h"

namespace next {
namespace fuxi {

// data field placement
// free offset/id queue + data storage zone
template <typename T>
class FlatObjectQueue {
 public:
  FlatObjectQueue();
  ~FlatObjectQueue();

  bool Initialize(size_t num, const ::next::fuxi::PlacementStorage& storage);

  T* Get();
  int Put(T* obj);

  uint32_t FreeNum() const { return free_ids_.FreeNum(); }

 public:
  static void GetStorageSchema(StorageSchema& schema, size_t num) {
    schema.fields.resize(2);
    schema.fields[0].field_id = 0;
    schema.fields[0].repeated_size = num;
    schema.fields[0].request_space_size =
        RteRing<uint32_t>::PlacementStorageSize(num);
    schema.fields[1].field_id = 1;
    schema.fields[1].repeated_size = num;
    schema.fields[1].request_space_size = sizeof(T) * num;
  }

  // reader side
  bool SetPlacementStorage(PlacementStorage& storage) { return true; }

 private:
  bool inited_{false};
  size_t queue_capacity_{0};

  // initial state is a full object pool
  RteRing<int32_t> free_ids_;  // which has a slot buffer
  T* storage_{nullptr};
};

// class impl
template <typename T>
FlatObjectQueue<T>::FlatObjectQueue() {}

template <typename T>
FlatObjectQueue<T>::~FlatObjectQueue() {}

template <typename T>
bool FlatObjectQueue<T>::Initialize(
    size_t num, const ::next::fuxi::PlacementStorage& storage) {
  if (inited_) {
    return true;
  }
  if (storage.field_buffers.size() != 2) {
    return false;
  }

  // placement new per pooled object
  storage_ = reinterpret_cast<T*>(storage.field_buffers[1].ptr);
  for (size_t i = 0; i < num; ++i) {
    new (&storage_[i]) T();
  }

  // init and fill free ids queue
  ::next::fuxi::PlacementStorage id_slot_storage;
  id_slot_storage.field_buffers.resize(1);
  id_slot_storage.field_buffers[0] = storage.field_buffers[0];
  free_ids_.Init(num, false, false, id_slot_storage);

  int32_t idx = 0;
  for (; idx < (int32_t)num; ++idx) {
    uint32_t ret = free_ids_.DoEnqueue(&idx, 1, nullptr);
    if (ret != 1) {
      // TODO: error
    }
  }

  queue_capacity_ = num;
  inited_ = true;
  return true;
}

template <typename T>
T* FlatObjectQueue<T>::Get() {
  int32_t id = -1;
  uint32_t ret = free_ids_.DoDequeue(&id, 1, nullptr);
  if (ret == 0) {
    return nullptr;
  }
  return &storage_[id];
}

template <typename T>
int FlatObjectQueue<T>::Put(T* obj) {
  int32_t idx = obj - storage_;
  if ((idx < 0) || (idx >= (int32_t)queue_capacity_)) {
    // TODO: error info
    return -1;
  }

  // TODO: check if has construct/destruct func
  obj->~T();
  new (obj) T();

  uint32_t ret = free_ids_.DoEnqueue(&idx, 1, nullptr);
  if (ret != 1) {
    // TODO: log error
    return -1;
  }

  return 0;
}

}  // namespace fuxi
}  // namespace next
