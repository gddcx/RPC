#include <string>
#include <vector>
#include <iostream>
#include "rpc_connector.h"
#include "rpc_protocol.h"

namespace crpc {

RpcConnector::RpcConnector(int netThread): _tcpClient(netThread) {
    _requestId = 0;

    _tcpClient.SetOnConnect(std::bind(&RpcConnector::_onConnectCallback, this, std::placeholders::_1));
    _tcpClient.SetOnMessage(std::bind(&RpcConnector::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpClient.SetOnClose(std::bind(&RpcConnector::_onCloseCallback, this, std::placeholders::_1));

    _tcpClient.InitClient(4096);
    _tcpClient.StartClient();
    _fd = _tcpClient.Connect("127.0.0.1", 50010);
}

void RpcConnector::_SafeMapInsert(std::pair<int, std::shared_ptr<std::promise<std::string>>> pair) {
    std::lock_guard<std::mutex> lock(_requestMapMutex);
    _requestMap.insert(pair);
}

void RpcConnector::_SafeMapDelete(int requestId) {
    std::lock_guard<std::mutex> lock(_requestMapMutex);
    _requestMap.erase(requestId);
}

void RpcConnector::_onConnectCallback(int fd) {

}

void RpcConnector::_onCloseCallback(int fd) {

}

void RpcConnector::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    RpcProtocol rpcProtocol;
    int httpHeaderLen = rpcProtocol.GetHttpHeaderLen();
    std::vector<char> data(httpHeaderLen, 0);
    if(recvBuf.GetBuffer(httpHeaderLen, data)) {
        rpcProtocol.ParseHttp(data);
    } else {
        return;
    }
    data.clear();

    int msgLen = rpcProtocol.GetMsgLen();
    data.resize(msgLen);
    std::cout << __func__ << "receive rsp" << std::endl;
    if(recvBuf.GetBuffer(msgLen, data)) {
        rpcProtocol.ParseMsg(data);
        int requestId = rpcProtocol.GetRequestId();
        std::cout << __func__ << "requestId = " << requestId << std::endl;
        std::cout << __func__ << "GetMsg() = " << rpcProtocol.GetMsg() << std::endl;
        _requestMap[requestId]->set_value(rpcProtocol.GetMsg());
        _SafeMapDelete(requestId);
    }
}

std::string RpcConnector::CallRemoteApi(char msgType, std::string& para) {
    int requestId = _requestId.fetch_add(1);
    std::cout << "requestId = " << requestId << std::endl;
    RpcProtocol::Build(requestId, msgType, para);
    _tcpClient.SendMsg(_fd, std::vector<char>(para.begin(), para.end()));

    auto promise = std::make_shared<std::promise<std::string>>();
    _SafeMapInsert(std::make_pair(requestId, promise));
    auto future = promise->get_future();

    std::string res = future.get(); // 阻塞调用
    return res;
}

}