#include <utility>
#include <iostream>
#include "rpc_service.h"

void RpcService::_SafeInsert(const std::string& serviceName, const std::string& serviceDest) {
    std::lock_guard<std::mutex> lock(_tblMutex);
    _serviceTbl[serviceName].push_back(serviceDest);
}

std::string RpcService::_SafeQuery(const std::string& serviceName) {
    std::lock_guard<std::mutex> lock(_tblMutex);
    if(_serviceTbl.find(serviceName) != _serviceTbl.end()) {
        return _serviceTbl[serviceName][0]; // TODO：缺少负载均衡，直接取了第一个主机来提供服务
    }

    return "None";
}
/* TODO:服务端注册新IP后，需要watcher去更新客户端本地缓存 */
void RpcService::RegisterService(const std::string& serviceName, const std::string& serviceDest) {
    _SafeInsert(serviceName, serviceDest);
}

std::string RpcService::QueryService(const std::string& sertiveName) {
    return _SafeQuery(sertiveName);
}