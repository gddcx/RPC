#include <vector>
#include "rpc_acceptor.h"

namespace crpc{

RpcAcceptor::RpcAcceptor(int netThreadNum): _tcpServer(netThreadNum) {
    _tcpServer.SetOnConnect(std::bind(&RpcAcceptor::_onConnectCallback, this, std::placeholders::_1));
    _tcpServer.SetOnMessage(std::bind(&RpcAcceptor::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpServer.SetOnClose(std::bind(&RpcAcceptor::_onCloseCallback, this, std::placeholders::_1));

    _tcpServer.InitServer(50002, 4096);
    _tcpServer.StartServer();
}

void RpcAcceptor::_onConnectCallback(int fd) {

}

void RpcAcceptor::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    RpcProtocol rpcProtocol;
    std::vector<char> data(rpcProtocol.commHeaderLen, 0);
    if(recvBuf.GetBuffer(rpcProtocol.commHeaderLen, data)) {
        rpcProtocol.ParseHeader(data);
    } else {
        return;
    }
    data.clear();
    data.resize(rpcProtocol.protoMsgLen);
    if(rpcProtocol.protoMsgLen == 0) {
        return;
    }
    if(recvBuf.GetBuffer(rpcProtocol.protoMsgLen, data)) {
        rpcProtocol.ParseBody(data);
        Process(fd, rpcProtocol.protoUUID, rpcProtocol.serviceIndex, rpcProtocol.serializePara);
    }
}

void RpcAcceptor::_onCloseCallback(int fd) {
    _tcpServer.Disconnection(fd);
}

bool RpcAcceptor::SendMsg(int fd, const std::string& data) {
    _tcpServer.SendMsg(fd, std::vector<char>(data.begin(), data.end()));
    return true;
}

}