
#pragma once

#include "fuxi/base/pthread_rw_lock.h"
#include "fuxi/common/shm_object.h"
#include "fuxi/shm/impl/span.h"
#include "fuxi/shm/topo/shm_meta.h"

namespace next {
namespace fuxi {

struct ReaderIndex {
  int32_t writer_idx{-1};
  int32_t reader_idx{-1};
};

// writer --> reader slot
struct ReaderSlot {
  bool is_online{false};
  uint64_t ts{0};
  uint32_t pid{0};
  int32_t process_state_index{-1};

  int32_t writer_id{-1};
  int32_t reader_id{-1};

  // index to reader process event queue
  uint32_t history_qos{1};
};

struct WriterSlot {
  bool is_online{false};
  char name[ShmMeta::MAX_DATA_NAME_LENGTH];
  uint32_t name_len{0};
  uint64_t name_id{0};
  int32_t writer_id{0};
  int32_t process_state_index{-1};

  // reader slot register management
  uint16_t reader_num{0};
  ReaderSlot reader_slots[ShmMeta::MAX_READER_NUM];

  bool SetName(const std::string& name);
  bool IsEquel(const std::string& a) const;
};

// global namespace
struct TopoData {
  // shm domain level topo state
  PthreadRWLock mutex;

  // record writer and process reader per writer
  uint16_t writer_num{0};
  WriterSlot writer_slot_table[ShmMeta::MAX_WRITER_NUM];
};

struct ReaderOption {
  std::string channel;
  std::string process_name;
  int32_t pid{-1};
  int32_t writer_id{-1};
  int32_t reader_id{-1};
  uint32_t history_qos{1};
};

class TopoState {
 public:
  TopoState();
  ~TopoState();

  int32_t RegisterWriter(int32_t process_idx, const std::string& name);
  ReaderIndex RegisterReader(int32_t process_idx, const std::string& name);

  // dataplane
  WriterSlot* GetWriterSlot(int32_t writer_index) {
    return &topo_data_.writer_slot_table[writer_index];
  }

 private:
  int32_t RegisterWriterLocked(int32_t process_idx, const std::string& name);
  int32_t RegisterReaderLocked(int32_t process_idx, uint32_t writer_idx);
  TopoData topo_data_;
};

}  // namespace fuxi
}  // namespace next
