
#include "fuxi/shm/impl/segment_state.h"

#include "fuxi/common/log.h"

namespace next {
namespace fuxi {

SegmentState::SegmentState() {}

SegmentState::~SegmentState() {}

bool SegmentState::SetSegmentSpan(uint64_t base_addr, uint64_t size) {
  if (IsInit()) {
    return true;
  }
  segment_base_addr_.store(base_addr);
  segment_size_.store(size);
  return true;
}

uint64_t SegmentState::GetSize() const { return segment_size_.load(); }

}  // namespace fuxi
}  // namespace next
