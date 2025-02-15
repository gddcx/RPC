#ifndef _SERVICE_DISCOVERY_H_
#define _SERVICE_DISCOVERY_H_

#include <string>
#include <utility>
#include <vector>

class ServiceDiscovery {
public:
    static std::string Build(uint16_t serviceIndex, uint8_t msgType, const std::vector<std::pair<uint32_t, uint16_t>>& serviceDest);
    void ParseHeader(const std::vector<char>& buffer);
    void ParseBody(const std::vector<char>& buffer);
public:
    static const int headerLen = 5;
    static const int ipPortUnitLen = 4 + 2;
    enum keeperMsgType{Msg_Register, Msg_Query};
    uint8_t msgType = 0;
    uint16_t serviceIndex;
    short bodyLen = 0;
    std::vector<std::pair<uint32_t, uint16_t>> serviceDest;
};

#endif