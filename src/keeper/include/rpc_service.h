#ifndef _RPC_SERVICE_H_
#define _RPC_SERVICE_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <mutex>

class RpcService {
private:
    void _SafeInsert(uint16_t serviceIndex, const std::pair<uint32_t, uint16_t>& ipAndPort);
    const std::vector<std::pair<uint32_t, uint16_t>>& _SafeQuery(uint16_t serviceIndex);
private:
    std::mutex _tblMutex;
    std::unordered_map<uint16_t, std::vector<std::pair<uint32_t, uint16_t>>> _serviceTbl; // 服务名称: [服务IP:PORT]
public:
    void RegisterService(uint16_t serviceIndex, const std::vector<std::pair<uint32_t, uint16_t>>& serviceDest);
    const std::vector<std::pair<uint32_t, uint16_t>>& QueryService(uint16_t serviceIndex);
};

#endif