set(GLOG_ROOT "${CMAKE_SOURCE_DIR}/3rd/glog")
set(GLOG_INCLUDE_DIRS ${GLOG_ROOT}/include)
set(GLOG_LIBRARY_DIRS ${GLOG_ROOT}/lib)
set(GLOG_LIBRARIES glog)
include_directories(${GLOG_INCLUDE_DIRS})
link_directories(${GLOG_LIBRARY_DIRS})

install(DIRECTORY ${GLOG_INCLUDE_DIRS} DESTINATION .)
install(DIRECTORY ${GLOG_LIBRARY_DIRS} DESTINATION .)
