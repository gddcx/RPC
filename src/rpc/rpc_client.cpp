#include "client.h"

namespace crpc {

RpcClient::RpcClient(int netThread): rpcConnector(netThread) {}

void RpcClient::RpcFunc1(RPCMsg::Request& req, RPCMsg::Response& rsp, std::pair<uint32_t, uint16_t> ipPort) {
    std::string para = req.SerializeAsString();
    std::string res = rpcConnector.CallRemoteApi(0, para, ipPort);
    rsp.ParseFromString(res);
}

}
