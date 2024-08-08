cmake_minimum_required(VERSION 3.14)

project(fuxi VERSION 1.0.0)

set(INCLUDE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories(${INCLUDE_ROOT})

option(BUILD_DEPS "use FetchContent download and build dependencies" OFF)
if(BUILD_DEPS)
  MESSAGE(STATUS "build deps: ${BUILD_DEPS}")
  include(FetchContent)
  FetchContent_Declare(
    absl
    URL "https://github.com/abseil/abseil-cpp/archive/refs/tags/20230802.1.tar.gz"
    URL_HASH SHA256=987ce98f02eefbaf930d6e38ab16aa05737234d7afbab2d5c4ea7adbe50c28ed
  )
  set(ABSL_ENABLE_INSTALL ON)
  FetchContent_MakeAvailable(absl)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/Packages.cmake)

set_source_files_properties(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fuxi/mm/trick/arena.cc"
  PROPERTIES COMPILE_FLAGS "-fno-access-control")

file(GLOB FUXI_SRCS
  "src/fuxi/base/*.cc"
  "src/fuxi/common/*.cc"
  "src/fuxi/shm/*.cc"
  "src/fuxi/shm/arena/*.cc"
  "src/fuxi/shm/topo/*.cc"
  "src/fuxi/shm/impl/*.cc"
  "src/fuxi/shm/port/*.cc"
  "src/fuxi/mm/*.cc"
  "src/fuxi/mm/port/*.cc"
  "src/fuxi/mm/trick/*.cc"
)

add_library(fuxi SHARED
  ${FUXI_SRCS}
)
target_link_libraries(fuxi
  -ldl
  absl::base
  protobuf
)

add_subdirectory(proto)
add_subdirectory(demo)
