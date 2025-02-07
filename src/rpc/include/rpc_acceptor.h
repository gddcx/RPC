#ifndef _RPC_ACCEPTOR_H_
#define _RPC_ACCEPTOR_H_

#include <mutex>
#include <functional>
#include <vector>
#include "tcp_server.h"
#include "rpc_protocol.h"

namespace crpc {

class RpcAcceptor {
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& recvBuf);
    void _onCloseCallback(int fd);
public:
    RpcAcceptor(int netThreadNum);
    bool SendMsg(int fd, const std::string& data);
private:
    RpcProtocol _rpcProtocol;
    TcpServer _tpcServer;
public:
    std::function<void(int, int, uint8_t, std::string&)> Process;
};

}

#endif