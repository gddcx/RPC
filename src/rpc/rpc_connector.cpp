#include <string>
#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <stdexcept>
#include "rpc_connector.h"
#include "rpc_protocol.h"

namespace crpc {

RpcConnector::RpcConnector(int netThread): _tcpClient(netThread) {
    _requestId.store(0);

    _tcpClient.SetOnConnect(std::bind(&RpcConnector::_onConnectCallback, this, std::placeholders::_1));
    _tcpClient.SetOnMessage(std::bind(&RpcConnector::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpClient.SetOnClose(std::bind(&RpcConnector::_onCloseCallback, this, std::placeholders::_1));

    _tcpClient.InitClient(4096);
    _tcpClient.StartClient();
}

RpcConnector::~RpcConnector() {
    // _tcpClient.CloseClient();
}

void RpcConnector::_SafeExec(uint16_t requestId, std::string& para) {
    std::lock_guard<std::mutex> lock(_requestMapMutex);
    if(_requestMap.find(requestId) == _requestMap.end()) {
        std::cout << "request id error, requestId:" << requestId << std::endl;
        return;
    }
    _requestMap[requestId]->set_value(para);
}

void RpcConnector::_SafeMapInsert(std::pair<uint16_t, std::shared_ptr<std::promise<std::string>>> pair) {
    std::lock_guard<std::mutex> lock(_requestMapMutex);
    _requestMap.insert(pair);
}

void RpcConnector::_SafeMapDelete(uint16_t requestId) {
    std::lock_guard<std::mutex> lock(_requestMapMutex);
    _requestMap.erase(requestId);
}

void RpcConnector::_onConnectCallback(int fd) {

}

void RpcConnector::_onCloseCallback(int fd) {
    {
        std::lock_guard<std::mutex> lock(_hasConnMutex);
        _getFdFromIpPort.erase(_getIpPortFromFd[fd]);
        _getIpPortFromFd.erase(fd);
    }
    _tcpClient.Disconnection(fd);
}

void RpcConnector::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    RpcProtocol rpcProtocol;
    int headerLen = rpcProtocol.commHeaderLen;
    std::vector<char> data(headerLen, 0);
    if(recvBuf.GetBuffer(headerLen, data)) {
        rpcProtocol.ParseHeader(data);
    } else {
        std::cout << "fd:" << fd << " can not get completed header. len:" << headerLen << std::endl;
        return;
    }

    data.clear();

    int msgLen = rpcProtocol.protoMsgLen;
    data.resize(msgLen);
    if(recvBuf.GetBuffer(msgLen, data)) {
        rpcProtocol.ParseBody(data);
        _SafeExec(rpcProtocol.protoUUID, rpcProtocol.serializePara);
        _SafeMapDelete(rpcProtocol.protoUUID);
    } else {
        std::cout << "fd:" << fd << " can not get completed body, len:" << msgLen << std::endl;
    }
}

std::string RpcConnector::CallRemoteApi(uint16_t serviceIndex, std::string& para, std::pair<uint32_t, uint16_t> ipPort) {
    int fd = 0;
    {
        std::lock_guard<std::mutex> lock(_hasConnMutex);
        if(_getFdFromIpPort.find(ipPort) != _getFdFromIpPort.end()) {
            fd = _getFdFromIpPort[ipPort];
        } else {
            sockaddr_in addr;
            addr.sin_addr.s_addr = htonl(ipPort.first);
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ipStr, INET_ADDRSTRLEN);
            fd = _tcpClient.Connect(ipStr, ipPort.second);
            if(fd < 0) {
                std::cout << __func__ << "connect failed" << std::endl;
                return "";
            }
            _getFdFromIpPort.insert(std::make_pair(ipPort, fd));
            _getIpPortFromFd.insert(std::make_pair(fd, ipPort));
        }
    }

    uint16_t requestId = _requestId.fetch_add(1);

    auto promise = std::make_shared<std::promise<std::string>>();
    _SafeMapInsert(std::make_pair(requestId, promise));
    auto future = promise->get_future();

    std::string data = RpcProtocol::Build(requestId, serviceIndex, para);
    _tcpClient.SendMsg(fd, std::vector<char>(data.begin(), data.end()));

    if(future.wait_for(std::chrono::seconds(3)) == std::future_status::ready) {
        return future.get(); // 阻塞调用
    }

    throw std::runtime_error("RPC Timeout");
}

}