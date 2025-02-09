#ifndef _RPC_SERVICE_H_
#define _RPC_SERVICE_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

class RpcService {
private:
    void _SafeInsert(const std::string& serviceName, const std::string& serviceDest);
    std::string _SafeQuery(const std::string& serviceName);
private:
    std::mutex _tblMutex;
    std::unordered_map<std::string, std::vector<std::string>> _serviceTbl; // 服务名称: [服务IP:PORT]
public:
    void RegisterService(const std::string& serviceName, const std::string& serviceDest);
    std::string QueryService(const std::string& serriveName);
};

#endif