#include "keeper_client.h"
#include <iostream>
#include "keeper_protocol.h"

KeeperClient::KeeperClient(int netThread): _tcpClient(netThread) {
    _tcpClient.SetOnConnect(std::bind(&KeeperClient::_onConnectCallback, this, std::placeholders::_1));
    _tcpClient.SetOnMessage(std::bind(&KeeperClient::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpClient.SetOnClose(std::bind(&KeeperClient::_onCloseCallback, this, std::placeholders::_1));

    _tcpClient.InitClient(4096);
    _tcpClient.StartClient();
    _fd = _tcpClient.Connect("127.0.0.1", 50011);
}

void KeeperClient::RegisterService(const std::string& serviceName, const std::string& ipAddr, const std::string& port) {
    std::string data = KeeperProtocol::Build(serviceName, KeeperProtocol::Msg_Register, ipAddr + ":" + port);
    _tcpClient.SendMsg(_fd, std::vector<char>(data.begin(), data.end()));
}

void KeeperClient::FetchService(const std::string& serviceName, std::string& ipAddr, std::string& port) {
    std::string data = KeeperProtocol::Build(serviceName, KeeperProtocol::Msg_Query, "");
    _tcpClient.SendMsg(_fd, std::vector<char>(data.begin(), data.end()));
}

void KeeperClient::_onConnectCallback(int fd) {

}

void KeeperClient::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    KeeperProtocol keeperProtocol;
    std::vector<char> data(keeperProtocol.headerLen, 0);
    if(recvBuf.GetBuffer(keeperProtocol.headerLen, data)) {
        keeperProtocol.ParseHeader(data);
    } else {
        return;
    }

    data.clear();
    data.resize(keeperProtocol.bodyLen);
    if(recvBuf.GetBuffer(keeperProtocol.bodyLen, data)) {
        keeperProtocol.ParseBody(data);
    }

    if(keeperProtocol.msgType == KeeperProtocol::Msg_Query) {
        size_t pos = keeperProtocol.serviceDest.find(":");
        std::string ipAddr = keeperProtocol.serviceDest.substr(0, pos);
        std::string port =keeperProtocol.serviceDest.substr(pos + 1, keeperProtocol.serviceDest.size());
        _SafeInsert(keeperProtocol.serviceName, ipAddr, atoi(port.c_str()));
    }

}

void KeeperClient::_onCloseCallback(int fd) {
    
}

void KeeperClient::_SafeInsert(const std::string& serviceName, const std::string& ipAddr, int port) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    _serviceCache.insert(std::make_pair(serviceName, std::make_pair(ipAddr, port)));
}