
#include "fuxi/shm/shm_domain.h"

#include "fuxi/shm/port/xsi_segment.h"

namespace next {
namespace fuxi {

ShmDomain::ShmDomain() {}

ShmDomain::~ShmDomain() {}

bool ShmDomain::Initialize(int32_t domain_id) {
  if (inited_) {
    return true;
  }

  if (false == metadata_.Init(domain_id)) {
    return false;
  }

  process_idx_ = RegisterProcess();
  if (-1 == process_idx_) {
    std::cout << "register process idx failed" << std::endl;
    return false;
  }

  inited_ = true;
  return true;
}

int32_t ShmDomain::RegisterProcess() {
  int32_t pid = getpid();
  metadata_.RegisterPid(pid);
  return 0;
}

ReaderIndex ShmDomain::RegisterReader(const std::string& name) {
  return metadata_.RegisterReader(process_idx_, name);
}

int32_t ShmDomain::RegisterWriter(const std::string& name) {
  return metadata_.RegisterWriter(process_idx_, name);
}

std::shared_ptr<Segment> ShmDomain::GetSegment(SegmentOption& opt) {
  int32_t idx = metadata_.layout->RegisterSegment(opt.name);

  // TODO: when domain init, reserved per segment virtual address space
  // which is PROT_NONE and madvise MADV_DONTDUMP

  // idx
  uint64_t seg_addr =
      ShmMeta::SHM_SEGMENTDATA_BASE_ADDR + idx * ShmMeta::SHM_MAX_SEGMENT_SIZE;
  opt.base_addr = (void*)seg_addr;
  // opt.space_size =   ;  // TODO:
  std::shared_ptr<Segment> segment = std::make_shared<XsiSegment>(opt);
  return segment;
}

bool ShmDomain::Push(int32_t writer_id,
                     const std::shared_ptr<ShmObject>& shm_obj) {
  WriterSlot* writer_slot = metadata_.GetWriterSlot(writer_id);

  // get all readers process idx
  // notify each process reader
  std::cout << "reader num " << writer_slot->reader_num << std::endl;
  for (size_t i = 0; i < writer_slot->reader_num; ++i) {
    ReaderSlot* reader_slot = &(writer_slot->reader_slots[i]);
    if (reader_slot->is_online == false) {
      continue;
    }

    int32_t process_index = reader_slot->process_state_index;
    ProcessState* process_state = metadata_.GetProcessState(process_index);
    if (process_state) {
      std::cout << "push to process index " << process_index << " reader idx "
                << reader_slot->process_state_index << std::endl;
      process_state->Push(shm_obj);
    }
  }
  return true;
}

std::shared_ptr<ShmObject> ShmDomain::Pop() {
  if (process_idx_ < 0) {
    std::cout << "process idex " << process_idx_ << std::endl;
    return nullptr;
  }

  ProcessState* process_state = metadata_.GetProcessState(process_idx_);
  return process_state->Pop();
}

std::shared_ptr<ShmObject> ShmDomain::TryPop() {
  if (process_idx_ < 0) {
    std::cout << "process idex " << process_idx_ << std::endl;
    return nullptr;
  }

  // TODO: use try mode
  ProcessState* process_state = metadata_.GetProcessState(process_idx_);
  return process_state->Pop();
}

}  // namespace fuxi
}  // namespace next
