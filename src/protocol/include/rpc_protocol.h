#ifndef _RPC_PROTOCOL_H_
#define _RPC_PROTOCOL_H_

#include <vector>
#include "protocol_comm.h"
/*
    TODO: 使用HTTP1.0/2.0
    ----------------------+-------------+
    | serviceIndex(2byte) |     para    |
    ----------------------+-------------+
*/

class RpcProtocol: public ProtocolComm {
public:
    static std::string Build(uint16_t uuid, uint16_t serviceIndex, std::string& para);
    void ParseHeader(const std::vector<char>& data);
    void ParseBody(const std::vector<char>& data);
public:
    uint16_t serviceIndex;
    std::string serializePara;
};

#endif