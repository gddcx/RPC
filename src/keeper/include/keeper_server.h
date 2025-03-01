#ifndef _KEEPER_SERVER_H_
#define _KEEPER_SERVER_H_

#include <atomic>
#include "tcp_server.h"
#include "rpc_service.h"
#include "service_discovery.h"
#include "heart_beat_protocol.h"
#include "logging.h"

using namespace crpc;
class KeeperServer {
private:
    TcpServer _tcpServer;
    RpcService _rpcService;
    std::atomic<uint16_t> _uuid;
    Logger _logger;
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& buffer);
    void _onCloseCallback(int fd);
    void _onServiceDiscovery(int fd, const ProtocolComm& protocolComm,const ServiceDiscovery& serviceDiscovery);
    void _onHeartBeat(int fd, const ProtocolComm& protocolComm, const HeartBeatProtocol& heartBeat);
public:
    KeeperServer();
};

#endif