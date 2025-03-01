#include <iostream>
#include "protocol_comm.h"

std::string ProtocolComm::BulidComm(uint8_t type, uint16_t id, uint16_t len) {
    std::string buffer(5, 0);

    buffer[0] = type;
    buffer[1] = id & 0xff;
    buffer[2] = (id >> 8) & 0xff;
    buffer[3] = len & 0xff;
    buffer[4] = (len >> 8) & 0xff;

    return buffer;
}

void ProtocolComm::ParseComm(const std::vector<char>& data) {
    protoMsgType = data[0];
    protoUUID = (data[2] << 8) | data[1];
    protoMsgLen = (data[4] << 8) | data[3];
}