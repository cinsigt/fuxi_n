#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <atomic>

namespace next {
namespace fuxi {

#define FUXI_DO_IF(condition, code) \
  if (condition) {                  \
    code;                           \
  }

#define FUXI_DO_IF_NULL(condition, code) \
  if (condition == nullptr) {            \
    code;                                \
  }

#if !defined(RETURN_IF)
#define RETURN_IF(condition) \
  if (condition) {           \
    return;                  \
  }
#endif

#if !defined(_RETURN_VAL_IF_NULL2__)
#define _RETURN_VAL_IF_NULL2__
#define RETURN_VAL_IF_NULL2(ptr, val) \
  if (ptr == nullptr) {               \
    return (val);                     \
  }
#endif

#if !defined(_RETURN_VAL_IF2__)
#define _RETURN_VAL_IF2__
#define RETURN_VAL_IF2(condition, val) \
  if (condition) {                     \
    return (val);                      \
  }
#endif

}  // namespace fuxi
}  // namespace next
