#include "keeper_server.h"
#include "keeper_protocol.h"

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
    KeeperProtocol keeperProtocol;
    std::vector<char> data(keeperProtocol.headerLen, 0);
    if(buffer.GetBuffer(keeperProtocol.headerLen, data)) {
        keeperProtocol.ParseHeader(data);
    } else {
        return;
    }

    data.clear();
    data.resize(keeperProtocol.bodyLen);
    if(buffer.GetBuffer(keeperProtocol.bodyLen, data)) {
        keeperProtocol.ParseBody(data);
    }

    switch(keeperProtocol.msgType) {
        case KeeperProtocol::Msg_Register:
        {
            _rpcService.RegisterService(keeperProtocol.serviceName, keeperProtocol.serviceDest);
            break;
        }
        case KeeperProtocol::Msg_Query:
        {
            std::string hostInfo = _rpcService.QueryService(keeperProtocol.serviceName);
            std::string rsp = KeeperProtocol::Build(keeperProtocol.serviceName, KeeperProtocol::Msg_Query, hostInfo);
            _tcpServer.SendMsg(fd, std::vector<char>(rsp.begin(), rsp.end()));
            break;
        }
    }
}

void KeeperServer::_onCloseCallback(int fd) {

}
