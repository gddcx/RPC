#ifndef _SERVICE_DISCOVERY_H_
#define _SERVICE_DISCOVERY_H_

#include <string>
#include <utility>
#include <vector>
#include <unordered_set>
#include "protocol_comm.h"

/* 

+--------------------------------+--------------------------------+---------------------------------------------+
|      serviceIndex (2byte)      |          Port (2byte)          |                    IP(4byte)                |
+--------------------------------+--------------------------------+---------------------------------------------+

*/

struct SetCmp {
    template <typename T1, typename T2>
    size_t operator()(const std::pair<T1, T2>& p) const {
        return std::hash<T1>{}(p.first) ^ std::hash<T2>{}(p.second);
    }
};

class ServiceDiscovery: public ProtocolComm {
public:
    static std::string Build(uint8_t msgType, uint16_t id, uint16_t serviceIdx);
    static std::string Build(uint8_t msgType, uint16_t id, uint16_t serviceIdx, const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& serviceDest);
    void ParseHeader(const std::vector<char>& buffer);
    void ParseBody(const std::vector<char>& buffer);
public:
    static constexpr int ipPortUnitLen = 4 + 2;
    uint16_t serviceIndex;
    std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp> serviceDest;
};

#endif