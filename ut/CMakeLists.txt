cmake_minimum_required(VERSION 3.16.0)
project(test)

set(CMAKE_CXX_FLAGS "-std=c++14 -pthread -Wall -O0 -g")

include_directories(${ROOT_DIR}/rpc/)
include_directories(${ROOT_DIR}/rpc/network)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


add_executable(network_ut network.cpp)
target_link_libraries(network_ut ${LIB_NET})
target_link_libraries(network_ut ${GTEST_LIBRARIES} pthread)
