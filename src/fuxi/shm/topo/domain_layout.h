
#pragma once

#include <string>

#include "fuxi/shm/impl/segment.h"
#include "fuxi/shm/impl/span.h"
#include "fuxi/shm/topo/process_state.h"
#include "fuxi/shm/topo/shm_meta.h"
#include "fuxi/shm/topo/topo_state.h"

namespace next {
namespace fuxi {

// proto header
struct ShmProtoHeader {
  uint32_t header_space_size = 0;
  uint32_t version = 0;
  uint64_t timestamp = 0;
  PthreadRWLock mutex;
  std::atomic<uint8_t> inited{false};
  int32_t domain_id = 0;
  uint32_t total_space_size = 0;
  uint32_t free_space_size = 0;
  uint32_t free_space_used = 0;

  // field
  inline size_t PlacementStorageSize() const { return 0; }
};

// segment field
struct SegmentIndicator {
  char name[ShmMeta::MAX_DATA_NAME_LENGTH];
  uint32_t name_len = 0;
  int32_t index = 0;
  uint64_t base_addr = 0;
  uint64_t timestamp = 0;
  int32_t pid = 0;
  uint32_t segment_size = 0;

  std::atomic<bool> allocated{false};
  std::atomic<bool> is_valid{false};

  bool SetName(const std::string& name);
  bool IsEquel(const std::string& name) const;
};

struct SegmentIndicatorTable {
  PthreadRWLock mutex;
  std::atomic<uint16_t> segment_num{0};
  SegmentIndicator indicator_table[ShmMeta::MAX_SEGMENT_NUM];

  inline size_t PlacementStorageSize() const { return 0; }

  // interface:
  int32_t RegisterSegment(const std::string& name);
  bool GetSegment(SegmentIndicator& meta, int32_t seg_index);
};

// process state field
struct ProcessStateTable {
  PthreadRWLock mutex;
  uint16_t process_cnt{0};
  ProcessState process_state[ShmMeta::MAX_PROCESS_NUM];

  static inline size_t PlacementStorageSize() {
    StorageSchema schema;
    ProcessState::GetStorageSchema(schema, ShmMeta::MAX_READER_EVENT_SIZE);
    return ShmMeta::MAX_PROCESS_NUM * schema.TotalSize();
  }
};

// layout define
struct ShmDomainLayout {
  ShmProtoHeader header;                // not need placement
  TopoState topo_state;                 // not need placement
  SegmentIndicatorTable segment_table;  // not need placement
  ProcessStateTable process_table;      // need placement
  uint8_t data[0];

  // func
  bool Init(void* addr, uint32_t nbytes, int32_t domain_id);
  bool WaitUntilReady() const;

  bool Allocate(const StorageSchema& schema, PlacementStorage& storage);
  void* Allocate(size_t size);

  // ctrl interface
  int32_t RegisterPid(int32_t pid);

  ReaderIndex RegisterReader(int32_t process_idx, const std::string& name);
  int32_t RegisterWriter(int32_t process_idx, const std::string& name);

  int32_t RegisterSegment(const std::string& name);

  ProcessState* GetProcessState(int32_t process_index);

  static size_t TotalSpaceSize();
};

}  // namespace fuxi
}  // namespace next
