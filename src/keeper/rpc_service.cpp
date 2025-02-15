#include <iostream>
#include <stdexcept>
#include "rpc_service.h"

void RpcService::_SafeInsert(uint16_t serviceIndex, const std::pair<uint32_t, uint16_t>& ipAndPort) {
    std::lock_guard<std::mutex> lock(_tblMutex);
    _serviceTbl[serviceIndex].push_back(ipAndPort);
}

const std::vector<std::pair<uint32_t, uint16_t>>& RpcService::_SafeQuery(uint16_t serviceIndex) {
    std::lock_guard<std::mutex> lock(_tblMutex);
    if(_serviceTbl.find(serviceIndex) != _serviceTbl.end()) {
        return _serviceTbl[serviceIndex];
    }

    throw std::runtime_error("No service");
}
/* TODO:服务端注册新IP后，需要watcher去更新客户端本地缓存 */
void RpcService::RegisterService(uint16_t serviceIndex, const std::vector<std::pair<uint32_t, uint16_t>>& serviceDest) {
    for(size_t idx = 0; idx < serviceDest.size(); idx++) {
        _SafeInsert(serviceIndex, serviceDest[idx]);
    }
}

const std::vector<std::pair<uint32_t, uint16_t>>& RpcService::QueryService(uint16_t serviceIndex) {
    return _SafeQuery(serviceIndex);
}