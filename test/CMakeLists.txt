cmake_minimum_required(VERSION 3.16.0)
project(test)

set(CMAKE_CXX_FLAGS "-std=c++2a -pthread -Wall -O0 -g")

include_directories(${ROOT_DIR}/rpc/)
include_directories(${ROOT_DIR}/rpc/network)
include_directories(${ROOT_DIR}/src/keeper/include)
include_directories(${ROOT_DIR}/src/monitoring/include)
include_directories(${ROOT_DIR}/src/network/include)
include_directories(${ROOT_DIR}/src/protocol/include)
include_directories(${ROOT_DIR}/src/rpc/include)
include_directories(${ROOT_DIR}/src/utils/include)

include(protobuf.cmake)

find_package(nlohmann_json REQUIRED)

add_executable(rpc_client_main rpc_client_main.cpp)
target_link_libraries(rpc_client_main nlohmann_json::nlohmann_json ${LIB_NET} ${LIB_UTILS} ${LIB_RPC} ${LIB_PROTOCOL} ${LIB_KEEPER} ${LIB_MONITOR}
absl::check 
absl::flags
absl::flags_parse
absl::log
${_REFLECTION} 
${_GRPC_GRPCPP} 
${_PROTOBUF_LIBPROTOBUF}
)

add_executable(rpc_server_main rpc_server_main.cpp)
target_link_libraries(rpc_server_main nlohmann_json::nlohmann_json ${LIB_NET} ${LIB_RPC} ${LIB_PROTOCOL} ${LIB_KEEPER} ${LIB_UTILS} ${LIB_MONITOR}
absl::check 
absl::flags
absl::flags_parse
absl::log
${_REFLECTION} 
${_GRPC_GRPCPP} 
${_PROTOBUF_LIBPROTOBUF}
)

add_executable(monitor monitoring_main.cpp)
target_link_libraries(monitor nlohmann_json::nlohmann_json ${LIB_NET} ${LIB_RPC} ${LIB_PROTOCOL} ${LIB_KEEPER} ${LIB_UTILS} ${LIB_MONITOR})

add_executable(keeper keeper_main.cpp)
target_link_libraries(keeper nlohmann_json::nlohmann_json ${LIB_NET} ${LIB_RPC} ${LIB_PROTOCOL} ${LIB_KEEPER} ${LIB_UTILS} ${LIB_MONITOR})