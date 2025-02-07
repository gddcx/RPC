#include <vector>
#include "rpc_acceptor.h"

namespace crpc{

RpcAcceptor::RpcAcceptor(int netThreadNum): _tpcServer(netThreadNum) {
    _tpcServer.SetOnConnect(std::bind(&RpcAcceptor::_onConnectCallback, this, std::placeholders::_1));
    _tpcServer.SetOnMessage(std::bind(&RpcAcceptor::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tpcServer.SetOnClose(std::bind(&RpcAcceptor::_onCloseCallback, this, std::placeholders::_1));

    _tpcServer.InitServer(50010, 4096);
    _tpcServer.StartServer();
}

void RpcAcceptor::_onConnectCallback(int fd) {

}

void RpcAcceptor::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    int httpHeaderLen = _rpcProtocol.GetHttpHeaderLen();
    std::vector<char> data(httpHeaderLen, 0);
    if(recvBuf.GetBuffer(httpHeaderLen, data)) {
        _rpcProtocol.ParseHttp(data);
    } else {
        return;
    }
    data.clear();
    int msgLen = _rpcProtocol.GetMsgLen();
    data.resize(msgLen);
    if(recvBuf.GetBuffer(msgLen, data)) {
        _rpcProtocol.ParseMsg(data);
        Process(fd, _rpcProtocol.GetRequestId(), _rpcProtocol.GetMsgType(), _rpcProtocol.GetMsg());
    }
}

void RpcAcceptor::_onCloseCallback(int fd) {
    _tpcServer.Disconnection(fd);
}

bool RpcAcceptor::SendMsg(int fd, const std::string& data) {
    _tpcServer.SendMsg(fd, std::vector<char>(data.begin(), data.end()));
    return true;
}

}