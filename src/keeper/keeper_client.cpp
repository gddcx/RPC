#include <utility>
#include <iostream>
#include <unistd.h>
#include "keeper_client.h"

KeeperClient::KeeperClient(const std::string& serverIP, uint16_t port, int netThread): _tcpClient(netThread) {
    _tcpClient.SetOnConnect(std::bind(&KeeperClient::_onConnectCallback, this, std::placeholders::_1));
    _tcpClient.SetOnMessage(std::bind(&KeeperClient::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpClient.SetOnClose(std::bind(&KeeperClient::_onCloseCallback, this, std::placeholders::_1));

    _tcpClient.InitClient(4096);
    _tcpClient.StartClient();
    _fd = _tcpClient.Connect(serverIP, port);
    while(_fd == -1) {
        std::cout << __FUNCTION__ << ">>> connect to " << serverIP << ":" << port << "fail" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        _fd = _tcpClient.Connect(serverIP, port);
    }

    _uuid.store(0);
}

KeeperClient::~KeeperClient() {
    if(_fd != -1) close(_fd);
}

void KeeperClient::RegisterService(uint16_t serviceIndex, uint32_t ipAddr, uint16_t port) {
    uint16_t uuid = _uuid.fetch_add(1);
    std::string data = ServiceDiscovery::Build(MessageType::FUNC_REGISTER, uuid, serviceIndex, std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>{{ipAddr, port}});
    _tcpClient.SendMsg(_fd, std::vector<char>(data.begin(), data.end()));
}

std::future<std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>> KeeperClient::FetchService(uint16_t serviceIndex) {
    uint16_t uuid = _uuid.fetch_add(1);
    auto promise = std::make_shared<std::promise<std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>>>();
    _SafeInsert(serviceIndex, promise);
    std::string data = ServiceDiscovery::Build(MessageType::FUNC_QUERY, uuid, serviceIndex);
    _tcpClient.SendMsg(_fd, std::vector<char>(data.begin(), data.end()));
    return promise->get_future();
}

void KeeperClient::_onConnectCallback(int fd) {

}

void KeeperClient::_onCloseCallback(int fd) {
    _tcpClient.Disconnection(fd);
}

void KeeperClient::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    ServiceDiscovery serviceDiscovery;
    std::vector<char> data(serviceDiscovery.commHeaderLen, 0);
    if(recvBuf.GetBuffer(serviceDiscovery.commHeaderLen, data)) {
        serviceDiscovery.ParseHeader(data);
    } else {
        return;
    }

    data.clear();
    data.resize(serviceDiscovery.protoMsgLen);
    if(recvBuf.GetBuffer(serviceDiscovery.protoMsgLen, data)) {
        serviceDiscovery.ParseBody(data);
    }

    if(serviceDiscovery.protoMsgType == MessageType::FUNC_QUERY) {
        _SafeExec(serviceDiscovery.serviceIndex, serviceDiscovery.serviceDest);
        _SafeErase(serviceDiscovery.serviceIndex);
    } else if(serviceDiscovery.protoMsgType == MessageType::FUNC_REGISTER) {

    }
}

void KeeperClient::_SafeExec(uint16_t serviceIndex, std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& dest) {
    std::lock_guard<std::mutex> lock(_taskSyncTblMutex);
    if(_taskSyncTbl.find(serviceIndex) != _taskSyncTbl.end()) {
        _taskSyncTbl[serviceIndex]->set_value(dest);
    }
}

void KeeperClient::_SafeInsert(uint16_t serviceIndex, std::shared_ptr<std::promise<std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>>> promise) {
    std::lock_guard<std::mutex> lock(_taskSyncTblMutex);
    _taskSyncTbl.emplace(serviceIndex, promise);
}

void KeeperClient::_SafeErase(uint16_t serviceIndex) {
    std::lock_guard<std::mutex> lock(_taskSyncTblMutex);
    _taskSyncTbl.erase(serviceIndex);
}