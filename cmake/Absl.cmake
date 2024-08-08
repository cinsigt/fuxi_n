if(NOT TARGET absl::base)
  MESSAGE(STATUS "no absl base")
  find_package(absl REQUIRED)
endif()
