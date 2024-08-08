
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "fuxi/shm/impl/segment.h"
#include "fuxi/shm/topo/domain_layout.h"
#include "fuxi/shm/topo/domain_metadata.h"

namespace next {
namespace fuxi {

struct SegmentHandle {
  std::shared_ptr<Segment> segment;
};

// per process has a shm domain instance
class ShmDomain {
 public:
  ShmDomain();
  ~ShmDomain();

  bool Initialize(int32_t domain_id);
  int32_t RegisterProcess();
  ReaderIndex RegisterReader(const std::string& name);
  int32_t RegisterWriter(const std::string& name);

  // backend shared memory resource
  std::shared_ptr<Segment> GetSegment(SegmentOption& opt);

  // process level signal send
  bool Push(int32_t writer_id, const std::shared_ptr<ShmObject>& shm_obj);

  // process level signal receive
  std::shared_ptr<ShmObject> Pop();
  std::shared_ptr<ShmObject> TryPop();

 private:
  bool inited_{false};
  std::string ns_{"shm_domain"};
  int32_t domain_id_{0};

  // system level used domain flat data
  DomainMetadata metadata_;

  int32_t process_idx_{-1};

  // local process used segment
  std::mutex ctrl_mutex_;
  ProcessState* local_process_state_{nullptr};
  std::unordered_map<std::string, SegmentHandle> output_segment_map_;
  std::unordered_map<std::string, SegmentHandle> input_segment_map_;
};
using ShmDomainPtr = std::shared_ptr<ShmDomain>;

}  // namespace fuxi
}  // namespace next
