#ifndef _KEEPER_CLIENT_H_
#define _KEEPER_CLIENT_H_

#include <string>
#include <unordered_map>
#include <future>
#include <mutex>
#include <atomic>
#include "tcp_client.h"
#include "rpc_service.h"
#include "service_discovery.h"

using namespace crpc;
class KeeperClient {
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& recvBuf);
    void _onCloseCallback(int fd);
    void _SafeInsert(uint16_t serviceIndex, std::shared_ptr<std::promise<std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>>> promise);
    void _SafeErase(uint16_t serviceIndex);
public:
    KeeperClient(const std::string& serverIP, uint16_t port, int netThread);
    ~KeeperClient();
    void RegisterService(uint16_t serviceIndex, uint32_t ipAddr, uint16_t port);
    std::future<std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>> FetchService(uint16_t serviceIndex);
private:
    int _fd = -1;
    std::atomic<uint16_t> _uuid;
    TcpClient _tcpClient;
    std::mutex _taskSyncTblMutex;
    std::unordered_map<uint16_t, std::shared_ptr<std::promise<std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>>>> _taskSyncTbl;
};

#endif