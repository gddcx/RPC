#include "keeper_server.h"
#include "service_discovery.h"

KeeperServer::KeeperServer(): _tcpServer(1) {
    _tcpServer.SetOnConnect(std::bind(&KeeperServer::_onConnectCallback, this, std::placeholders::_1));
    _tcpServer.SetOnMessage(std::bind(&KeeperServer::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpServer.SetOnClose(std::bind(&KeeperServer::_onCloseCallback, this, std::placeholders::_1));

    _tcpServer.InitServer(50011, 4096);
    _tcpServer.StartServer();
}

void KeeperServer::_onConnectCallback(int fd) {

}

void KeeperServer::_onMessageCallback(int fd, RecvBuffer& buffer) {
    ServiceDiscovery serviceDiscovery;
    std::vector<char> data(serviceDiscovery.headerLen, 0);
    if(buffer.GetBuffer(serviceDiscovery.headerLen, data)) {
        serviceDiscovery.ParseHeader(data);
    } else {
        return;
    }

    data.clear();
    data.resize(serviceDiscovery.bodyLen);
    if(buffer.GetBuffer(serviceDiscovery.bodyLen, data)) {
        serviceDiscovery.ParseBody(data);
    }

    switch(serviceDiscovery.msgType) {
        case ServiceDiscovery::Msg_Register:
        {
            _rpcService.RegisterService(serviceDiscovery.serviceIndex, serviceDiscovery.serviceDest);
            break;
        }
        case ServiceDiscovery::Msg_Query:
        {
            try {
                const std::vector<std::pair<uint32_t, uint16_t>>& hostInfo = _rpcService.QueryService(serviceDiscovery.serviceIndex);
                std::string rsp = ServiceDiscovery::Build(serviceDiscovery.serviceIndex, ServiceDiscovery::Msg_Query, hostInfo);
                _tcpServer.SendMsg(fd, std::vector<char>(rsp.begin(), rsp.end()));
            } catch(const std::runtime_error& e) {
                std::cout << "Runtime error:" << e.what() << std::endl;
            }
            break;
        }
    }
}

void KeeperServer::_onCloseCallback(int fd) {

}
