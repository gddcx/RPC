#ifndef _KEEPER_PROTOCOL_H_
#define _KEEPER_PROTOCOL_H_

#include <string>

class KeeperProtocol {
public:
    static std::string Build(const std::string& serviceName, uint8_t msgType, const std::string& serviceDest);
    void ParseHeader(const std::vector<char>& data);
    void ParseBody(const std::vector<char>& data);
public:
    static const int headerLen = 3;
    int bodyLen = 0;
    uint8_t msgType = 0;
    enum keeperMsgType{Msg_Register, Msg_Query};
    std::string serviceName;
    std::string serviceDest;
};

#endif