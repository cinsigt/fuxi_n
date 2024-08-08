
#include "fuxi/shm/topo/topo_state.h"

#include <string.h>

namespace next {
namespace fuxi {

//
bool WriterSlot::SetName(const std::string& a) {
  if (a.size() > sizeof(name)) {
    return false;
  }

  bzero(name, sizeof(name));
  strncpy(name, a.c_str(), a.size());
  name_len = a.size();
  return true;
}

bool WriterSlot::IsEquel(const std::string& a) const {
  if (name_len != a.size()) {
    return false;
  }

  return (strncmp(name, a.c_str(), name_len) == 0);
}

// class TopoState
TopoState::TopoState() {}

TopoState::~TopoState() {}

int32_t TopoState::RegisterWriter(int32_t process_idx,
                                  const std::string& name) {
  if (name.size() > ShmMeta::MAX_DATA_NAME_LENGTH) {
    return -1;
  }

  WriteLockGuard<PthreadRWLock> lock(topo_data_.mutex);
  int32_t idx = RegisterWriterLocked(process_idx, name);
  if (idx >= 0) {
    // set writer online
    topo_data_.writer_slot_table[idx].is_online = true;
  }
  return idx;
}

int32_t TopoState::RegisterWriterLocked(int32_t process_idx,
                                        const std::string& name) {
  for (uint16_t i = 0; i < topo_data_.writer_num; ++i) {
    if (topo_data_.writer_slot_table[i].IsEquel(name)) {
      return i;
    }
  }

  if (topo_data_.writer_num >= ShmMeta::MAX_READER_NUM) {
    return -1;
  }
  int32_t idx = topo_data_.writer_num++;
  topo_data_.writer_slot_table[idx].SetName(name);
  return idx;
}

ReaderIndex TopoState::RegisterReader(int32_t process_idx,
                                      const std::string& name) {
  ReaderIndex idx;
  if (name.size() > ShmMeta::MAX_DATA_NAME_LENGTH) {
    return idx;
  }

  WriteLockGuard<PthreadRWLock> lock(topo_data_.mutex);
  int32_t writer_idx = -1;
  writer_idx = RegisterWriterLocked(process_idx, name);
  std::cout << "reader register writer idx " << writer_idx << std::endl;
  if (writer_idx == -1) {
    return idx;
  }

  idx.writer_idx = writer_idx;
  idx.reader_idx = RegisterReaderLocked(process_idx, writer_idx);
  if (idx.reader_idx >= 0) {
    // set reader online
    WriterSlot* slot = &topo_data_.writer_slot_table[writer_idx];
    slot->reader_slots[idx.reader_idx].is_online = true;
  }
  return idx;
}

int32_t TopoState::RegisterReaderLocked(int32_t process_idx,
                                        uint32_t writer_idx) {
  WriterSlot* slot = &topo_data_.writer_slot_table[writer_idx];
  for (int32_t i = 0; i < slot->reader_num; ++i) {
    if (process_idx == slot->reader_slots[i].process_state_index) {
      return i;
    }
  }

  if (slot->reader_num >= ShmMeta::MAX_READER_NUM) {
    return -1;
  }
  int32_t idx = slot->reader_num++;
  slot->reader_slots[idx].process_state_index = process_idx;
  slot->reader_slots[idx].is_online = true;
  return idx;
}

}  // namespace fuxi
}  // namespace next
