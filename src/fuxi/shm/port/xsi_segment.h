
#pragma once

#include "fuxi/shm/impl/segment.h"

namespace next {
namespace fuxi {

class XsiSegment : public Segment {
 public:
  explicit XsiSegment(const SegmentOption& opt);
  virtual ~XsiSegment();

  static const char* Type() { return "xsi"; }

 private:
  void Reset() override;   // detach
  bool Remove() override;  // rm
  bool OpenOnly() override;
  bool OpenOrCreate() override;

  key_t key_ = -1;
  std::string path_;
};

}  // namespace fuxi
}  // namespace next
