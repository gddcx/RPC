#ifndef _RPC_SERVICE_H_
#define _RPC_SERVICE_H_

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <utility>
#include <mutex>
#include "service_discovery.h"

class RpcService {
private:
    void _SafeInsert(uint16_t serviceIndex, const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& ipAndPort);
    const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& _SafeQuery(uint16_t serviceIndex);
private:
    std::mutex _tblMutex;
    std::unordered_map<uint16_t, std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>> _serviceTbl; // 服务名称: [服务IP:PORT]
public:
    void RegisterService(uint16_t serviceIndex, const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& serviceDest);
    const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& QueryService(uint16_t serviceIndex);
};

#endif