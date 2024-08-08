#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <atomic>

#include "fuxi/base/ring_common.h"

namespace next {
namespace fuxi {

class RingBuffer {
 public:
  RingBuffer();
  RingBuffer(uint32_t min_size, bool is_sp, bool is_sc);
  ~RingBuffer();

  bool Init(uint32_t min_size, bool is_sp, bool is_sc);

  uint32_t DoEnqueue(void* const* obj_table, uint32_t n,
                     uint32_t* free_space = nullptr, bool is_sp = true);
  uint32_t DoDequeue(void** obj_table, uint32_t n,
                     uint32_t* available = nullptr, bool is_sc = true);

  inline uint32_t Size() const { return size_; }

 private:
  uint32_t MoveProdHead(uint32_t n, uint32_t* old_head, uint32_t* new_head,
                        uint32_t* free_entries, bool is_sp);
  uint32_t MoveConsHead(uint32_t n, uint32_t* old_head, uint32_t* new_head,
                        uint32_t* entries, bool is_sc);
  int EnqueuePtrs(void** ring_start, uint32_t prod_head, void* const* obj_table,
                  uint32_t n);
  int DequeuePtrs(void** ring_start, uint32_t cons_head, void** obj_table,
                  uint32_t n);

  uint32_t size_ = 0;
  uint32_t mask_ = 0;
  uint32_t capacity_ = 0;
  bool is_sp_ = true;
  bool is_sc_ = true;
  bool inited_ = false;

  alignas(128) struct ring_headtail prod_;
  alignas(128) struct ring_headtail cons_;

  alignas(128) void** ring_ = nullptr;
};

}  // namespace fuxi
}  // namespace next
