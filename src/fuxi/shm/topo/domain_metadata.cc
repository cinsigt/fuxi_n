
#include "fuxi/shm/topo/domain_metadata.h"

#include "fuxi/mm/port/shm_option.h"

namespace next {
namespace fuxi {

bool DomainMetadata::Init(int32_t domain_id) {
  if (inited) {
    return true;
  }

  size_t pagesize = sysconf(_SC_PAGESIZE);

  ShmOption opt;
  opt.domain_id = domain_id;
  opt.name_id = 8000;
  // opt.path = ;
  opt.name = "domain_topo";
  // opt.addr = 0x56414bf11000;
  opt.addr = (void*)ShmMeta::DOMAIN_METADATA_BASE_ADDR;
  opt.size = ShmDomainLayout::TotalSpaceSize();

  std::cout << "domain layout total size " << opt.size << std::endl;

  // set opt
  if (false == shm.Init(opt)) {
    return false;
  }

  if (shm.IsCreated() == false) {
    // wait layout finished
    layout = reinterpret_cast<ShmDomainLayout*>(shm.BaseAddr());
    layout->WaitUntilReady();
    return true;
  }

  // now only one reach here, and init layout

  layout = reinterpret_cast<ShmDomainLayout*>(shm.BaseAddr());
  layout->Init(shm.BaseAddr(), shm.SpaceSize(), domain_id);
  return true;
}

int32_t DomainMetadata::RegisterPid(int32_t pid) {
  return layout->RegisterPid(pid);
}

int32_t DomainMetadata::RegisterWriter(int32_t process_idx,
                                       const std::string& name) {
  return layout->RegisterWriter(process_idx, name);
}

ReaderIndex DomainMetadata::RegisterReader(int32_t process_idx,
                                           const std::string& name) {
  return layout->RegisterReader(process_idx, name);
}

// dataplane
WriterSlot* DomainMetadata::GetWriterSlot(int32_t writer_index) {
  return layout->topo_state.GetWriterSlot(writer_index);
}

ProcessState* DomainMetadata::GetProcessState(int32_t process_index) {
  return layout->GetProcessState(process_index);
}

}  // namespace fuxi
}  // namespace next
