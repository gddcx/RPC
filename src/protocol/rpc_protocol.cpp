#include <algorithm>
#include <iostream>
#include "protocol_comm.h"
#include "rpc_protocol.h"

std::string RpcProtocol::Build(uint16_t uuid, uint16_t serviceIdx, std::string& para) {
    uint16_t bodyLen = sizeof(serviceIdx) + para.size();
    std::string header = ProtocolComm::BulidComm(RPC, uuid, bodyLen);

    return header + std::string((char*)&serviceIdx, sizeof(serviceIdx)) + para;
}

void RpcProtocol::ParseHeader(const std::vector<char>& data) {
    ProtocolComm::ParseComm(data);
}

void RpcProtocol::ParseBody(const std::vector<char>& data) {
    serviceIndex = ((uint16_t)data[1] << 8) | (uint16_t)data[0];
    serializePara = std::string(data.begin() + sizeof(serviceIndex), data.end());
}