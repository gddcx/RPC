#include <string>
#include <chrono>
#include <arpa/inet.h>
#include "rpc_server.h"
#include "rpc_invoker.h"

namespace crpc {

RpcServer::RpcServer(int netThread, int serviceThread): _rpcProcessor(serviceThread), _rpcAcceptor(netThread), _logger("rpc_server.log") {
    _rpcProcessor.SetAcceptor(&_rpcAcceptor);
}

RpcServer::~RpcServer() {
    delete _keeperClient;
}

void RpcServer::SetKeeper(const std::string& ip, uint16_t port) {
    _keeperClient = new KeeperClient(ip, port, 1);
    _logger.Log(LOG_INFO, "connect to keeper %s:%d", ip.c_str(), port);
}

std::unordered_map<uint16_t, std::function<std::string(std::string)>> RpcServer::_IDToFunc = {
    {Test_Func, TestFunc},
};

void RpcServer::RpcFuncSet() {
    in_addr registerCent;
    inet_pton(AF_INET, "127.0.0.1", &registerCent);
    int ipAddr = ntohl(registerCent.s_addr);
    int port = 50002;
    
    if(_keeperClient == nullptr) {
        std::cout << __FUNCTION__ << ">>> _keeperClient is not initialized" << std::endl;
        return;
    }

    for(int idx = 0; idx < RpcFuncNum; ++idx) {
        _keeperClient->RegisterService(idx, ipAddr, port);
        _rpcProcessor.MessageRegister(idx, _IDToFunc[idx]);
        _logger.Log(LOG_INFO, "register service %d", idx);
    }
}

int RpcServer::Main() {
    RpcFuncSet();

    _timer.AddTimer(5555, [&](){
        _ipcMsgQueue.SendMsg(IPCMsg{1, 8});
    }, true);
    _timer.RunTimer();
    
    return 0;
}

}