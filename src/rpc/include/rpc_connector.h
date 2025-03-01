#ifndef _RPC_CONNECTOR_H_
#define _RPC_CONNECTOR_H_

#include <unordered_map>
#include <utility>
#include <future>
#include <string>
#include <atomic>
#include <utility>
#include "tcp_client.h"

namespace crpc {

struct PairCmp {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        return std::hash<T1>{}(pair.first) & std::hash<T2>{}(pair.second);
    }
};

class RpcConnector {
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& recvBuf);
    void _onCloseCallback(int fd);
    void _SafeMapInsert(std::pair<int, std::shared_ptr<std::promise<std::string>>>);
    void _SafeMapDelete(int requestId);
public:
    RpcConnector(int netThread);
    ~RpcConnector();
    std::string CallRemoteApi(uint16_t serviceIndex, std::string& para, std::pair<uint32_t, uint16_t> ipPort);
private:
    std::mutex _requestMapMutex;
    std::unordered_map<int, std::shared_ptr<std::promise<std::string>>> _requestMap;
    std::mutex _hasConnMutex;
    std::unordered_map<std::pair<uint32_t, uint16_t>, int, PairCmp> _getFdFromIpPort;
    std::unordered_map<int, std::pair<uint32_t, uint16_t>> _getIpPortFromFd;
public:
    std::atomic<int> _requestId;
    TcpClient _tcpClient;
};

}

#endif