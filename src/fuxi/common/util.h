#pragma once

#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>
#include <string>

#include "fuxi/common/message_header.h"

namespace next {
namespace fuxi {

class Util {
 public:
  static inline const std::string GetEnv(const std::string& var_name) {
    std::string val_str = "";

    char* val = std::getenv(var_name.c_str());
    if (val != nullptr) {
      val_str = val;
    }
    return val_str;
  }

  static bool EnsureDirectory(const std::string& directory_path);
  static bool DirectoryExists(const std::string& directory_path);

  static void Convert32ToArray(uint32_t input, char* output) {
    uint32_t n = htonl(input);
    memcpy(static_cast<void*>(output), static_cast<const void*>(&n), sizeof(n));
  }

  static void Convert64ToArray(uint64_t input, char* output) {
    uint32_t h_high =
        static_cast<uint32_t>((input & 0xffffffff00000000UL) >> 32);
    uint32_t h_low = static_cast<uint32_t>(input & 0x00000000ffffffffUL);
    Convert32ToArray(h_high, output);
    Convert32ToArray(h_low, output + 4);
  }

  static uint32_t ConvertArrayTo32(const char* input) {
    uint32_t n = 0;
    memcpy(static_cast<void*>(&n), static_cast<const void*>(input), sizeof(n));
    return ntohl(n);
  }

  static uint64_t ConvertArrayTo64(const char* input) {
    uint64_t high = ConvertArrayTo32(input);
    uint64_t low = ConvertArrayTo32(input + 4);
    return (high << 32) | low;
  }
};

}  // namespace fuxi
}  // namespace next
