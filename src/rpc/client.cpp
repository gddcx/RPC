#include "client.h"

namespace crpc {

RpcClient::RpcClient(int netThread): rpcConnector(netThread) {}

void RpcClient::RpcFunc1(RPCMsg::Request& req, RPCMsg::Response& rsp) {
    std::string para = req.SerializeAsString();
    std::string res = rpcConnector.CallRemoteApi(1, para);
    rsp.ParseFromString(res);
}

}
