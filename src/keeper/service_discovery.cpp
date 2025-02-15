#include <vector>
#include "service_discovery.h"


std::string ServiceDiscovery::Build(uint16_t serviceIndex, uint8_t msgType, const std::vector<std::pair<uint32_t, uint16_t>>& serviceDest) {
    int bufferSize = headerLen + serviceDest.size() * ipPortUnitLen;
    std::string buffer(bufferSize, 0);
    int offset = 0;
    /* header */
    buffer[offset++] = msgType;
    buffer[offset++] = serviceIndex & 0xff;
    buffer[offset++] = (serviceIndex >> 8) & 0xff;
    buffer[offset++] = bufferSize & 0xff;
    buffer[offset++] = (bufferSize >> 8) & 0xff;
    /* body */
    uint32_t ip;
    uint16_t port;
    for(size_t n = 0; n < serviceDest.size(); ++n) {
        ip = serviceDest[n].first;
        port = serviceDest[n].second;
        buffer[offset++] = port & 0xff;
        buffer[offset++] = (port >> 8) & 0xff;
        buffer[offset++] = ip & 0xff;
        buffer[offset++] = (ip >> 8) & 0xff;
        buffer[offset++] = (ip >> 16) & 0xff;
        buffer[offset++] = (ip >> 24) & 0xff;
    }

    return buffer;
}

void ServiceDiscovery::ParseHeader(const std::vector<char>& buffer) {
    msgType = buffer[0];
    serviceIndex = ((uint16_t)buffer[2] << 8) | buffer[1];
    bodyLen = ((uint16_t)buffer[4] << 8) | buffer[3];
}

void ServiceDiscovery::ParseBody(const std::vector<char>& data) {
    int hostNum = data.size() / ipPortUnitLen;
    serviceDest.reserve(hostNum);

    int baseOffset = 0;
    uint32_t ip;
    uint16_t port;
    for(int n = 0; n < hostNum; ++n) {
        baseOffset = n * 4;
        ip = ((uint32_t)data[baseOffset + 5] << 24) |
             ((uint32_t)data[baseOffset + 4] << 16) |
             ((uint32_t)data[baseOffset + 3] << 8)  |
             (uint32_t)data[baseOffset + 2];
        port = ((uint16_t)data[baseOffset + 1] << 8) | data[baseOffset + 0];
        serviceDest.emplace_back(ip, port);
    }
}