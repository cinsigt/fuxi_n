
#pragma once

#include <string>

namespace next {
namespace fuxi {

class ShmMeta {
 public:
  static constexpr size_t MAX_DATA_NAME_LENGTH = 256;

  static constexpr size_t MAX_PROCESS_NUM = 256;

  static constexpr size_t MAX_WRITER_NUM = 1024;
  static constexpr size_t MAX_READER_NUM =
      128;  // per writer -> multi reader connection
  static constexpr size_t MAX_READER_EVENT_SIZE = 512;

  static constexpr uint64_t SEGMENT_MAGIC_HEADER = 0x1F1F1F1F1F1F1F1FULL;
  static constexpr uint64_t EXTENT_MAGIC_HEADER = 0x2F2F2F2F2F2F2F2FULL;

  // unified addr range
  static constexpr uint64_t DOMAIN_METADATA_BASE_ADDR = 0x510000000000ul;
  static constexpr uint64_t SHM_SEGMENTDATA_BASE_ADDR =
      0x520000000000ull;  // ~ 0x5800 0000 0000ull
  static constexpr uint64_t SHM_MAX_SEGMENT_SIZE = 0x100000000ull;
  static constexpr size_t MAX_SEGMENT_NUM = 1024;
};

}  // namespace fuxi
}  // namespace next
