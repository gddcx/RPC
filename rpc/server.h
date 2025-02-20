#ifndef _SERVER_H_
#define _SERVER_H_

#include "rpc_acceptor.h"
#include "rpc_processor.h"
#include "keeper_client.h"

namespace crpc {

class RpcServer {
public:
    RpcServer(int netThread, int serviceThread);
    int Main();
private:
    RpcProcessor _rpcProcessor;
    RpcAcceptor _rpcAcceptor;
    KeeperClient _keeperClient;

};

}

#endif