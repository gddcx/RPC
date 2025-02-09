#include <vector>
#include "keeper_protocol.h"

std::string KeeperProtocol::Build(const std::string& serviceName, uint8_t type, const std::string& serviceDest) {
    std::string str = serviceName + " " + serviceDest;
    short len = str.size();
    std::vector<char> header(headerLen, 0);
    header[0] = len & 0xff;
    header[1] = (len >> 8) & 0xff;
    header[2] = type;
    return std::string(header.begin(), header.end()) + str;
}

void KeeperProtocol::ParseHeader(const std::vector<char>& data) {
    int offset = 0;
    int tmpData = 0;
    bodyLen = 0;
    for(; offset < headerLen-1; offset++) {
        tmpData = data[offset];
        bodyLen |= (tmpData << (offset * 8));
    }
    msgType = data[offset];
}

void KeeperProtocol::ParseBody(const std::vector<char>& data) {
    std::string str(data.begin(), data.end());
    size_t pos = str.find(" ");
    if(pos != str.npos) {
        serviceName = str.substr(0, pos);
        serviceDest = str.substr(pos + 1, str.size());
    }
}