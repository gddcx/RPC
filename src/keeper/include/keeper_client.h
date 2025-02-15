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
    void _SafeInsert(uint16_t serviceIndex, const std::pair<uint32_t, uint16_t>& ipAndPort);
public:
    KeeperClient(int netThread);
    void RegisterService(uint16_t serviceIndex, uint32_t ipAddr, uint16_t port);
    void FetchService(uint16_t serviceIndex, std::string& ipAddr, std::string& port);
private:
    int _fd;
    TcpClient _tcpClient;
    std::mutex _cacheMutex;
    std::unordered_map<uint16_t, std::vector<std::pair<uint32_t, uint16_t>>> _serviceCache;
};

#endif