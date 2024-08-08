message(INFO " : PROTOBUF_COMPILE")
set(PROTOBUF_ROOT "${CMAKE_SOURCE_DIR}/3rd/protobuf-3.18.1")
set(PROTOBUF_FOUND 1 CACHE INTERNAL "PROTOBUF_FOUND")
set(PROTOBUF_ROOT "${CMAKE_SOURCE_DIR}/3rd/protobuf-3.18.1" CACHE INTERNAL "PROTOBUF_ROOT")
set(PROTOBUF_LIBRARY_DIR ${PROTOBUF_ROOT}/output/lib CACHE INTERNAL "PROTOBUF_LIBRARY_DIR")
set(PROTOBUF_INCLUDE_DIR ${PROTOBUF_ROOT}/src CACHE INTERNAL "PROTOBUF_INCLUDE_DIR")
set(PROTOBUF_PROTOC_EXECUTABLE ${PROTOBUF_ROOT}/output/lib/protoc CACHE INTERNAL                    "PROTOBUF_PROTOC_EXECUTABLE")
set(PROTOC ${PROTOBUF_PROTOC_EXECUTABLE} CACHE INTERNAL "PROTOC")
 
 
include_directories(${PROTOBUF_INCLUDE_DIR})
link_directories(${PROTOBUF_LIBRARY_DIR})

function(PROTOBUF_GENERATE_CPP SRCS HDRS)

  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif(NOT ARGN)

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_PATH ${FIL} PATH)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    STRING(REGEX REPLACE "^(.*)\\.[^\\.]*$" "\\1" ABS_FIL_PRE ${ABS_FIL})
    # set(FIL_PATH "${CMAKE_SOURCE_DIR}/include/cybertron/proto/")
    set(ABS_FIL_PRE "${FIL_PATH}/${FIL_WE}")

    #list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${SRCS} "${ABS_FIL_PRE}.pb.cc")
    #list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")
    list(APPEND ${HDRS} "${ABS_FIL_PRE}.pb.h")

    add_custom_command(
        OUTPUT "${ABS_FIL_PRE}.pb.cc"
        "${ABS_FIL_PRE}.pb.h"
      COMMAND  ${PROTOC}
      ARGS --python_out ${FIL_PATH} --cpp_out ${FIL_PATH} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

function(PROTOBUF_GENERATE_CPP_IMPORT1 SRCS HDRS IMPORT_PATH1)

  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif(NOT ARGN)

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
               "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOC}
      ARGS --cpp_out ${CMAKE_CURRENT_BINARY_DIR} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} 
                                                 --proto_path "${CMAKE_CURRENT_SOURCE_DIR}/${IMPORT_PATH1}" 
                                                 ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

function(PROTOBUF_GENERATE_CPP_IMPORT2 SRCS HDRS IMPORT_PATH1 IMPORT_PATH2)

  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif(NOT ARGN)

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
               "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOC}
      ARGS --cpp_out ${CMAKE_CURRENT_BINARY_DIR} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} 
                                                 --proto_path "${CMAKE_CURRENT_SOURCE_DIR}/${IMPORT_PATH1}" 
                                                 --proto_path "${CMAKE_CURRENT_SOURCE_DIR}/${IMPORT_PATH2}" 
                                                 ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

install(DIRECTORY ${PROTOBUF_INCLUDE_DIR} DESTINATION .)
install(DIRECTORY ${PROTOBUF_LIBRARY_DIR} DESTINATION .)
