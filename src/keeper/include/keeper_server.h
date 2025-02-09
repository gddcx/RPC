#ifndef _KEEPER_SERVER_H_
#define _KEEPER_SERVER_H_

#include "tcp_server.h"
#include "rpc_service.h"

using namespace crpc;
class KeeperServer {
private:
    TcpServer _tcpServer;
    RpcService _rpcService;
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& buffer);
    void _onCloseCallback(int fd);
public:
    KeeperServer();
};

#endif