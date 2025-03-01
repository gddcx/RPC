#ifndef _SERVER_H_
#define _SERVER_H_

#include <unordered_map>
#include "rpc_acceptor.h"
#include "rpc_processor.h"
#include "keeper_client.h"
#include "timer.h"
#include "ipc_msg.h"
#include "logging.h"

namespace crpc {

class RpcServer {
private:
    void RpcFuncSet();
public:
    RpcServer(int netThread, int serviceThread);
    ~RpcServer();
    void SetKeeper(const std::string& ip, uint16_t port);
    int Main();
private:
    RpcProcessor _rpcProcessor;
    RpcAcceptor _rpcAcceptor;
    KeeperClient* _keeperClient = nullptr;
    Timer _timer;
    IPCMsgQueue _ipcMsgQueue;
    static std::unordered_map<uint16_t, std::function<std::string(std::string)>> _IDToFunc;
    enum RpcFunc {
        Test_Func,
        RpcFuncNum
    };
    Logger _logger;
};

}

#endif