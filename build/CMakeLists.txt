cmake_minimum_required(VERSION 3.16.0)
project(RPC)

set(CMAKE_CXX_FLAGS "-std=c++14 -pthread -Wall -O0 -g")

include_directories(${ROOT_DIR}/rpc)
include_directories(${ROOT_DIR}/rpc/network)
include_directories(${ROOT_DIR}/src/network/include)

include(network.cmake)