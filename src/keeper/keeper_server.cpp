#include <unistd.h>
#include "keeper_server.h"
#include "service_discovery.h"

KeeperServer::KeeperServer(): _tcpServer(1), _logger("keeper.log") {
    _tcpServer.SetOnConnect(std::bind(&KeeperServer::_onConnectCallback, this, std::placeholders::_1));
    _tcpServer.SetOnMessage(std::bind(&KeeperServer::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpServer.SetOnClose(std::bind(&KeeperServer::_onCloseCallback, this, std::placeholders::_1));

    _tcpServer.InitServer(50001, 4096);
    _tcpServer.StartServer();

    _logger.Log(LOG_INFO, "KeeperServer start. Listen on %d", 50001);
}

void KeeperServer::_onConnectCallback(int fd) {

}

void KeeperServer::_onCloseCallback(int fd) {
    _tcpServer.Disconnection(fd);
}

void KeeperServer::_onServiceDiscovery(int fd, const ProtocolComm& protocolComm, const ServiceDiscovery& serviceDiscovery) {
    switch(protocolComm.protoMsgType) {
        case MessageType::FUNC_REGISTER:
        {
            _rpcService.RegisterService(serviceDiscovery.serviceIndex, serviceDiscovery.serviceDest);

            _logger.Log(LOG_INFO, "Register service %d. ", serviceDiscovery.serviceIndex);
            break;
        }
        case MessageType::FUNC_QUERY:
        {
            try {
                const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& hostInfo = _rpcService.QueryService(serviceDiscovery.serviceIndex);
                std::string rsp = ServiceDiscovery::Build(MessageType::FUNC_QUERY, protocolComm.protoUUID, serviceDiscovery.serviceIndex, hostInfo);
                _tcpServer.SendMsg(fd, std::vector<char>(rsp.begin(), rsp.end()));

                _logger.Log(LOG_INFO, "Query service %d. ", serviceDiscovery.serviceIndex);
            } catch(const std::runtime_error& e) {
                std::cout << "Runtime error:" << e.what() << std::endl;
            }
            break;
        }
    }
}

void KeeperServer::_onHeartBeat(int fd, const ProtocolComm& protocolComm, const HeartBeatProtocol& heartBeat) {
    switch(protocolComm.protoMsgType) {
        case MessageType::PING: // 收到请求
        {
            std::string rsp = HeartBeatProtocol::Build(MessageType::PONG, protocolComm.protoUUID);
            _tcpServer.SendMsg(fd, std::vector<char>(rsp.begin(), rsp.end()));

            _logger.Log(LOG_INFO, "Receive ping from %d. ", protocolComm.protoUUID);
            break;
        }
        case MessageType::PONG: // 收到回应
        {
            break;
        }
    }
}

void KeeperServer::_onMessageCallback(int fd, RecvBuffer& buffer) {
    ProtocolComm protocolComm;
    std::vector<char> data(protocolComm.commHeaderLen, 0);
    if(buffer.GetBuffer(protocolComm.commHeaderLen, data)) {
        protocolComm.ParseComm(data);
    } else {
        return;
    }

    data.clear();
    data.resize(protocolComm.protoMsgLen);
    if(protocolComm.protoMsgType == MessageType::FUNC_REGISTER || protocolComm.protoMsgType == MessageType::FUNC_QUERY) {
        ServiceDiscovery serviceDiscovery;
        if(buffer.GetBuffer(protocolComm.protoMsgLen, data)) {
            serviceDiscovery.ParseBody(data);
            _onServiceDiscovery(fd, protocolComm, serviceDiscovery);
        }
    } else if(protocolComm.protoMsgType == MessageType::PING || protocolComm.protoMsgType == MessageType::PONG) {
        HeartBeatProtocol heartBeat;
        if(buffer.GetBuffer(protocolComm.protoMsgLen, data)) {
            heartBeat.ParseBody(data);
            _onHeartBeat(fd, protocolComm, heartBeat);
        }
    }

    
}

