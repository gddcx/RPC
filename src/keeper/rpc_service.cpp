#include <iostream>
#include <stdexcept>
#include "rpc_service.h"

void RpcService::_SafeInsert(uint16_t serviceIndex, const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& ipAndPort) {
    std::lock_guard<std::mutex> lock(_tblMutex);
    for(const auto& p: ipAndPort) {
        _serviceTbl[serviceIndex].insert(p);
    }
}

const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& RpcService::_SafeQuery(uint16_t serviceIndex) {
    std::lock_guard<std::mutex> lock(_tblMutex);
    if(_serviceTbl.find(serviceIndex) != _serviceTbl.end()) {
        return _serviceTbl[serviceIndex];
    }

    throw std::runtime_error("no this service");
}
/* TODO:服务端注册新IP后，需要watcher去更新客户端本地缓存 */
void RpcService::RegisterService(uint16_t serviceIndex, const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& serviceDest) {
    _SafeInsert(serviceIndex, serviceDest);
}

const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& RpcService::QueryService(uint16_t serviceIndex) {
    return _SafeQuery(serviceIndex);
}