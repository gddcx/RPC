#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "rpc_connector.h"
#include "rpc_api.h"

namespace crpc{

class RpcClient: public RpcApi {
public:
    RpcClient() = default;
    RpcClient(int netThread);
public:
    RpcConnector rpcConnector;
    void RpcFunc1(RPCMsg::Request& req, RPCMsg::Response& rsp, std::pair<uint32_t, uint16_t> ipPort) override;
};

}

#endif