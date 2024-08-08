
#pragma once

#include <string>

#include "fuxi/mm/port/xsi_shm.h"
#include "fuxi/shm/topo/domain_layout.h"

namespace next {
namespace fuxi {

struct DomainMetadata {
  bool inited{false};
  XsiShm shm;
  ShmDomainLayout* layout{nullptr};

  // func
  bool Init(int32_t domain_id);

  int32_t RegisterPid(int32_t pid);

  ReaderIndex RegisterReader(int32_t process_idx, const std::string& name);
  int32_t RegisterWriter(int32_t process_idx, const std::string& name);

  // dataplane
  WriterSlot* GetWriterSlot(int32_t writer_index);
  ProcessState* GetProcessState(int32_t process_index);
};

}  // namespace fuxi
}  // namespace next
