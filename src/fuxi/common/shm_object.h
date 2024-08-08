#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <atomic>

#include "fuxi/common/message_header.h"

namespace next {
namespace fuxi {

struct ShmMessage {
  MessageHeader header;

  // dynamic size payload
  uint8_t payload[0];
};

// shared object between multi processes
// two level reference eg: std::shared_ptr<ShmObject>
// 1. shared_ptr<> reference cnt is zero, check ShmObject reference cnt
// 2. then is ShmObject ref cnt is zero, return shm object and attached blocks
struct ShmObject {
  static constexpr uint32_t MAX_BLOCK_NUM = 32;
  MessageHeader header;

  uint64_t data_addr{0};

  int32_t ref_cnt{0};  // process level reference count

  uint16_t segment_id{0};  // reserved
  uint16_t extent_id{0};   // reserved

  uint32_t block_num{0};
  uint32_t block_index[ShmObject::MAX_BLOCK_NUM];

  inline ::std::atomic<int32_t>& reference_cnt() noexcept {
    return reinterpret_cast<::std::atomic<int32_t>&>(ref_cnt);
  }

  inline void IncreaseReferenceCount() {
    ::std::atomic<int32_t>& ref = reinterpret_cast<::std::atomic<int32_t>&>(ref_cnt);
    ref.fetch_add(1);
  }
};

struct ShmEvent {
  uint64_t shm_object_addr{0};  // ShmObject addr
};

}  // namespace fuxi
}  // namespace next
