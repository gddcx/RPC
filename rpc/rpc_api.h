#ifndef _RPC_API_H_
#define _RPC_API_H_

#include <string>
#include "msg.pb.h"

class RpcApi {
public:
    virtual void RpcFunc1(RPCMsg::Request& req, RPCMsg::Response& rsp) = 0;
};

#endif