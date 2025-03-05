#include <vector>
#include <iostream>
#include "service_discovery.h"


std::string ServiceDiscovery::Build(uint8_t msgType, uint16_t id, uint16_t serviceIdx) {
    int offset = 0;
    int bodyLen = sizeof(serviceIdx);
    std::string buffer(bodyLen, 0);

    /* header */
    std::string header = ProtocolComm::BulidComm(msgType, id, bodyLen);
    /* body */
    buffer[offset++] = serviceIdx & 0xff;
    buffer[offset++] = (serviceIdx >> 8) & 0xff;

    return header + buffer;
}

std::string ServiceDiscovery::Build(uint8_t msgType, uint16_t id, uint16_t serviceIdx, const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& serviceDest) {
    int offset = 0;
    int bodyLen = sizeof(serviceIdx) + serviceDest.size() * ipPortUnitLen;
    std::string buffer(bodyLen, 0);

    /* header */
    std::string header = ProtocolComm::BulidComm(msgType, id, bodyLen);
    /* body */
    buffer[offset++] = serviceIdx & 0xff;
    buffer[offset++] = (serviceIdx >> 8) & 0xff;

    uint32_t ip;
    uint16_t port;
    for(auto& p: serviceDest) {
        ip = p.first;
        port = p.second;
        buffer[offset++] = port & 0xff;
        buffer[offset++] = (port >> 8) & 0xff;
        buffer[offset++] = ip & 0xff;
        buffer[offset++] = (ip >> 8) & 0xff;
        buffer[offset++] = (ip >> 16) & 0xff;
        buffer[offset++] = (ip >> 24) & 0xff;

    }
    return header + buffer;
}

void ServiceDiscovery::ParseHeader(const std::vector<char>& buffer) {
    ParseComm(buffer);
}

void ServiceDiscovery::ParseBody(const std::vector<char>& data) {
    uint32_t ip;
    uint16_t port;
    int baseOffset = 0;
    int hostNum = (data.size() - sizeof(serviceIndex))/ ipPortUnitLen;

    serviceIndex = ((data[baseOffset + 1] & 0xff) << 8) | (data[baseOffset] & 0xff);
    for(int n = 0; n < hostNum; ++n) {
        baseOffset = n * ipPortUnitLen + sizeof(serviceIndex);
        ip = ((data[baseOffset + 5] & 0xff) << 24) |
             ((data[baseOffset + 4] & 0xff) << 16) |
             ((data[baseOffset + 3] & 0xff) << 8)  |
              (data[baseOffset + 2] & 0xff);
        port = (data[baseOffset + 1] << 8) | data[baseOffset + 0];
        serviceDest.emplace(ip, port);
    }

}