
set(NIO_AD_ROOT "${CMAKE_SOURCE_DIR}/..")

# list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

include(${CMAKE_SOURCE_DIR}/cmake/Options.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Absl.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Protobuf.cmake)

#include(${CMAKE_SOURCE_DIR}/cmake/GLog.cmake)
#include(${CMAKE_SOURCE_DIR}/cmake/Boost.cmake)

include_directories(${CMAKE_SOURCE_DIR})
