

#include "fuxi/shm/topo/domain_layout.h"

#include <string>

#include "fuxi/common/time.h"

namespace next {
namespace fuxi {

int32_t SegmentIndicatorTable::RegisterSegment(const std::string& name) {
  WriteLockGuard<PthreadRWLock> lock(mutex);

  for (uint16_t i = 0; i < segment_num; ++i) {
    if (indicator_table[i].IsEquel(name)) {
      return i;
    }
  }

  int32_t idx = segment_num++;
  indicator_table[idx].SetName(name);
  indicator_table[idx].index = idx;
  return idx;
}

bool SegmentIndicator::IsEquel(const std::string& a) const {
  if (name_len != a.size()) {
    return false;
  }

  return (strncmp(name, a.c_str(), name_len) == 0);
}

bool SegmentIndicator::SetName(const std::string& a) {
  if (a.size() > sizeof(name)) {
    return false;
  }

  bzero(name, sizeof(name));
  strncpy(name, a.c_str(), a.size());
  name_len = a.size();
  return true;
}

/*static*/ size_t ShmDomainLayout::TotalSpaceSize() {
  size_t sz = 0;
  sz = sizeof(ShmDomainLayout);

  // reserved data
  sz += ProcessStateTable::PlacementStorageSize();
  return sz;
}

bool ShmDomainLayout::Init(void* addr, uint32_t nbytes, int32_t domain_id) {
  // WriteLockGuard<PthreadRWLock> lock(header.mutex);
  if (header.inited.load()) {
    // already inited
    return true;
  }

  // init header
  header.header_space_size = sizeof(ShmDomainLayout);
  header.version = 0x1;
  header.timestamp = Time::Now().ToNanosecond();
  header.domain_id = domain_id;
  header.total_space_size = nbytes;
  header.free_space_size = nbytes - header.header_space_size;
  header.free_space_used = 0;

  new (&topo_state) TopoState();
  new (&segment_table) SegmentIndicatorTable();

  // fill pool
  new (&process_table) ProcessStateTable();

  // pre init process state table
  StorageSchema schema;
  ProcessState::GetStorageSchema(schema, ShmMeta::MAX_READER_EVENT_SIZE);
  for (size_t i = 0; i < ShmMeta::MAX_PROCESS_NUM; ++i) {
    Buffer buffer;
    buffer.ptr = Allocate(schema.TotalSize());
    if (nullptr == buffer.ptr) {
      // failed
      // Reset();
      return false;
    }
    buffer.size = schema.TotalSize();
    process_table.process_state[i].Init(0, ShmMeta::MAX_READER_EVENT_SIZE,
                                        buffer);
  }

  // set inited flag
  header.inited.store(true);
  std::cout << "total size " << header.total_space_size << " header size "
            << header.header_space_size << " free space size "
            << header.free_space_size << " free space used "
            << header.free_space_used << std::endl;
  return true;
}

bool ShmDomainLayout::Allocate(const StorageSchema& schema,
                               PlacementStorage& storage) {
  storage.clear();
  storage.resize(schema.FieldSize());

  for (size_t i = 0; i < schema.FieldSize(); ++i) {
    void* ptr = Allocate(schema.fields[i].request_space_size);
    if (ptr == nullptr) {
      std::cout << "allocate " << i << " failed" << std::endl;
      return false;
    }
    storage.MutableFieldBuffer(i).ptr = ptr;
    storage.MutableFieldBuffer(i).space_size =
        schema.fields[i].request_space_size;
  }
  return true;
}

void* ShmDomainLayout::Allocate(size_t size) {
  // check remain
  if (size + header.free_space_used > header.free_space_size) {
    std::cout << "allocate null domain layout" << std::endl;
    return nullptr;
  }

  void* ptr = &(data[0]) + header.free_space_used;
  header.free_space_used += size;
  return ptr;
}

bool ShmDomainLayout::WaitUntilReady() const {
  while (header.inited.load() == false) {
    std::this_thread::sleep_for(std::chrono::microseconds(1000));  // 1ms
  }

  return true;
}

int32_t ShmDomainLayout::RegisterPid(int32_t pid) {
  WriteLockGuard<PthreadRWLock> lock(process_table.mutex);
  for (size_t i = 0; i < process_table.process_cnt; ++i) {
    if (pid == process_table.process_state[i].GetPid()) {
      std::cout << "get registered pid " << pid << " index " << i << std::endl;
      return i;
    }
  }

  if (process_table.process_cnt >= ShmMeta::MAX_PROCESS_NUM) {
    // check exit pid
    return -1;
  }
  int32_t idx = process_table.process_cnt++;
  process_table.process_state[idx].SetPidOnline(pid, true);
  return 0;
}

ReaderIndex ShmDomainLayout::RegisterReader(int32_t process_idx,
                                            const std::string& name) {
  return topo_state.RegisterReader(process_idx, name);
}

int32_t ShmDomainLayout::RegisterWriter(int32_t process_idx,
                                        const std::string& name) {
  return topo_state.RegisterWriter(process_idx, name);
}

ProcessState* ShmDomainLayout::GetProcessState(int32_t process_index) {
  return &(process_table.process_state[process_index]);
}

int32_t ShmDomainLayout::RegisterSegment(const std::string& name) {
  int32_t idx = segment_table.RegisterSegment(name);
  return idx;
}

}  // namespace fuxi
}  // namespace next
