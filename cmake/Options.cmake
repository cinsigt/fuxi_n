set (build_version "1.0.0.0")
#set(ENABLE_WARNNING "OPEN")
option(BUILD_SHARED_LIBS "" ON)

#set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")
set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -fPIC -pipe") 
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wnon-virtual-dtor -Werror -Wdeprecated -Wdeprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wnon-virtual-dtor -Wdeprecated -Wdeprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-compare -Wunused-result -Wunused-value -Wextra -Wreturn-type -Wuninitialized")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -DNDEBUG " CACHE STRING "" FORCE)

#message("Build Type:" "${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS}")

set (CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install CACHE STRING "" FORCE)
set (UNIT_TEST_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/test/cybertron/unit_test CACHE STRING "" FORCE)
set (CYBER_UNIT_TEST_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/test/cyber/unit_test CACHE STRING "" FORCE)
set (COV_HOME ${CMAKE_SOURCE_DIR}/tools/ccover)
set (CMAKE_CXX_COMPILER_CCOVER ${COV_HOME}/bin/g++ CACHE STRING "" FORCE)

# if(DEFINED ENABLE_WARNNING)
#     message(INFO "ENABLE_WARNNING")
#     #-Wconversion-null -Wsign-compare -Wreturn-type -Wunused-variable -Wcpp")
#     add_definitions("-W -Wall")
# else(DEFINED ENABLE_WARNNING)
#     add_definitions(-w)
# endif()

