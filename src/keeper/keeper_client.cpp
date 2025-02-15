#include <utility>
#include <iostream>
#include "keeper_client.h"
#include "service_discovery.h"

KeeperClient::KeeperClient(int netThread): _tcpClient(netThread) {
    _tcpClient.SetOnConnect(std::bind(&KeeperClient::_onConnectCallback, this, std::placeholders::_1));
    _tcpClient.SetOnMessage(std::bind(&KeeperClient::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpClient.SetOnClose(std::bind(&KeeperClient::_onCloseCallback, this, std::placeholders::_1));

    _tcpClient.InitClient(4096);
    _tcpClient.StartClient();
    _fd = _tcpClient.Connect("127.0.0.1", 50011);
}

void KeeperClient::RegisterService(uint16_t serviceIndex, uint32_t ipAddr, uint16_t port) {
    std::string data = ServiceDiscovery::Build(serviceIndex, ServiceDiscovery::Msg_Register, std::vector<std::pair<uint32_t, uint16_t>>{std::make_pair(ipAddr, port)});
    _tcpClient.SendMsg(_fd, std::vector<char>(data.begin(), data.end()));
}

void KeeperClient::FetchService(uint16_t serviceIndex, std::string& ipAddr, std::string& port) {
    std::string data = ServiceDiscovery::Build(serviceIndex, ServiceDiscovery::Msg_Query, std::vector<std::pair<uint32_t, uint16_t>>());
    _tcpClient.SendMsg(_fd, std::vector<char>(data.begin(), data.end()));
}

void KeeperClient::_onConnectCallback(int fd) {

}

void KeeperClient::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    ServiceDiscovery serviceDiscovery;
    std::vector<char> data(serviceDiscovery.headerLen, 0);
    if(recvBuf.GetBuffer(serviceDiscovery.headerLen, data)) {
        serviceDiscovery.ParseHeader(data);
    } else {
        return;
    }

    data.clear();
    data.resize(serviceDiscovery.bodyLen);
    if(recvBuf.GetBuffer(serviceDiscovery.bodyLen, data)) {
        serviceDiscovery.ParseBody(data);
    }

    if(serviceDiscovery.msgType == ServiceDiscovery::Msg_Query) {
        for(auto& ipAndPort: serviceDiscovery.serviceDest) {
            _SafeInsert(serviceDiscovery.serviceIndex, ipAndPort);
        }
    }
}

void KeeperClient::_onCloseCallback(int fd) {
    
}

void KeeperClient::_SafeInsert(uint16_t serviceIndex, const std::pair<uint32_t, uint16_t>& ipAndPort) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    _serviceCache[serviceIndex].push_back(ipAndPort);
}