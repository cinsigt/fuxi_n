#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <atomic>

#include "fuxi/base/ring_common.h"
#include "fuxi/base/storage_schema.h"

namespace next {
namespace fuxi {

template <typename T>
class RteRing {
 public:
  RteRing();
  RteRing(uint32_t num, bool is_sp, bool is_sc);
  RteRing(uint32_t num, bool is_sp, bool is_sc,
          const PlacementStorage& placement_storage);
  ~RteRing();

  bool Init(uint32_t num, bool is_sp, bool is_sc);
  bool Init(uint32_t num, bool is_sp, bool is_sc,
            const PlacementStorage& placement_storage);

  uint32_t DoEnqueue(T const* obj_table, uint32_t n,
                     uint32_t* free_space = nullptr, bool is_sp = true);
  uint32_t DoDequeue(T* obj_table, uint32_t n, uint32_t* available = nullptr,
                     bool is_sc = true);

  inline uint32_t Size() const { return size_; }
  inline bool UseOuterBuffer() const { return use_outer_buffer_; }

  uint32_t FreeNum() const { return 0; }

 public:
  static size_t AlignmentSize();
  static size_t AlignedNum(size_t num);
  static size_t PlacementStorageSize(size_t num);

  static void GetStorageSchema(StorageSchema& schema, size_t num) {
    schema.fields.resize(1);
    schema.fields[0].field_id = 0;
    schema.fields[0].repeated_size = num;
    schema.fields[0].request_space_size = PlacementStorageSize(num);
  }

  bool SetPlacementStorage(PlacementStorage& storage) {
    if (storage.field_buffers.size() != 1) {
      return false;
    }

    storage_ = storage.field_buffers[0].ptr;
    return true;
  }

 private:
  uint32_t MoveProdHead(uint32_t n, uint32_t* old_head, uint32_t* new_head,
                        uint32_t* free_entries, bool is_sp);
  uint32_t MoveConsHead(uint32_t n, uint32_t* old_head, uint32_t* new_head,
                        uint32_t* entries, bool is_sc);
  int EnqueueValues(T* ring_start, uint32_t prod_head, T const* obj_table,
                    uint32_t n);
  int DequeueValues(T* ring_start, uint32_t cons_head, T* obj_table,
                    uint32_t n);

  uint32_t size_ = 0;
  uint32_t mask_ = 0;
  uint32_t capacity_ = 0;
  bool is_sp_ = true;
  bool is_sc_ = true;
  bool inited_ = false;

  alignas(128) struct ring_headtail prod_;
  alignas(128) struct ring_headtail cons_;

  alignas(128) T* storage_ = nullptr;
  bool use_outer_buffer_{false};
};

}  // namespace fuxi
}  // namespace next

#include "fuxi/base/rte_ring.hpp"
