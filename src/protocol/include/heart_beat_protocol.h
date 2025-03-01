#ifndef _HEART_BEAT_PROTOCOL_H_
#define _HEART_BEAT_PROTOCOL_H_

#include <string>
#include <vector>
#include <unordered_map>
#include "protocol_comm.h"

/* 

 0               1               2
 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+--------+------+-------+-------+---------------+
| PayLoad|   rate(%)    |PayLoad|    rate(%)    |
| Type(4)|    (8)       |Type(4)|       (8)     |
+--------+--------------+-------+---------------+

*/

class HeartBeatProtocol: public ProtocolComm{
private:
    static void _BuildPayLoad(std::vector<uint8_t>& data, std::vector<uint8_t>& scores);
public:
    static std::string Build(uint8_t msgType, uint32_t id, std::vector<uint8_t>& scores);
    static std::string Build(uint8_t msgType, uint32_t id);
    void ParseHeader(const std::vector<char>& data);
    void ParseBody(const std::vector<char>& data);
private:
    const static int _payLoadUnitBits = 12;
public:
    enum PayLoadType {INVALID, SCORE1};
    std::unordered_map<PayLoadType, uint8_t> payLoad;
};

#endif