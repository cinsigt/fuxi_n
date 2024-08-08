
# include this file, you can get
set(GTEST_ROOT "${CMAKE_SOURCE_DIR}/3rd/gtest")
# ${GTEST_INCLUDE_DIRS}
# ${GTEST_LIBRARIES}
# ${GTEST_MAIN_LIBRARIES}
set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)
set(GTEST_LIBRARY_DIRS ${GTEST_ROOT}/lib)
set(GTEST_LIBRARIES ${GTEST_LIBRARY_DIRS}/libgtest.a)
set(GTEST_MAIN_LIBRARIES ${GTEST_LIBRARY_DIRS}/libgtest_main.a)
include_directories(${GTEST_INCLUDE_DIRS})
link_directories(${GTEST_LIBRARY_DIRS})
