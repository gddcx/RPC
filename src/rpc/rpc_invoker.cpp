#include <string>
#include <iostream>
#include "rpc_invoker.h"

RPCMsg::Response RpcApi(RPCMsg::Request req) {
    std::cout << __func__ << ": UID = " << req.userid() << ", hostname = " << req.hostname() << std::endl;
    RPCMsg::Response rsp;
    rsp.set_stat(33);
    return rsp;
}

std::string TestFunc(std::string s) {
    RPCMsg::Request req;
    req.ParseFromString(s);
    RPCMsg::Response rsp = RpcApi(req);
    return rsp.SerializeAsString();
}