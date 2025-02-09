#ifndef _KEEPER_CLIENT_H_
#define _KEEPER_CLIENT_H_

#include <string>
#include <unordered_map>
#include <mutex>
#include <utility>
#include "tcp_client.h"
#include "rpc_service.h"

using namespace crpc;
class KeeperClient {
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& recvBuf);
    void _onCloseCallback(int fd);
    void _SafeInsert(const std::string& serviceName, const std::string& ipAddr, int port);
public:
    KeeperClient(int netThread);
    void RegisterService(const std::string& serviceName, const std::string& ipAddr, const std::string& port);
    void FetchService(const std::string& serviceName, std::string& ipAddr, std::string& port);
private:
    int _fd;
    TcpClient _tcpClient;
    std::mutex _cacheMutex;
    std::unordered_map<std::string, std::pair<std::string, int>> _serviceCache;
};

#endif