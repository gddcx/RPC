#ifndef _RPC_CONNECTOR_H_
#define _RPC_CONNECTOR_H_

#include <unordered_map>
#include <future>
#include <string>
#include <atomic>
#include <utility>
#include "tcp_client.h"

namespace crpc {

class RpcConnector {
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& recvBuf);
    void _onCloseCallback(int fd);
    void _SafeMapInsert(std::pair<int, std::shared_ptr<std::promise<std::string>>>);
    void _SafeMapDelete(int requestId);
public:
    RpcConnector(int netThread);
    std::string CallRemoteApi(char msgType, std::string& para);
private:
    std::mutex _requestMapMutex;
    std::unordered_map<int, std::shared_ptr<std::promise<std::string>>> _requestMap;
public:
    int _fd;
    std::atomic<int> _requestId;
    TcpClient _tcpClient;
};

}

#endif