#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <atomic>

namespace next {
namespace fuxi {

struct MessageHeader {
  char magic_num[4];
  char proto_version[2];  // version
  char content_type[2];   //
  char seq[8];            //
  char timestamp_ns[8];
  char src_id[8];
  char dst_id[8];
  char version[8];  //
  char content_size[4];
  char reserved[12];
};

}  // namespace fuxi
}  // namespace next
