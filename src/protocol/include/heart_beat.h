#ifndef _HEART_BEAT_H_
#define _HEART_BEAT_H_

#include <string>
#include <vector>

/* 
 0               1               2
 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+-+-------------+-------------------------------+
|O| Payload Len |         Session ID            |
|P|     (7)     |             (16)              |
|C|             |                               |
|O|             |                               |
|D|             |                               |
|E|             |                               |
+--------+------+-------+-------+---------------+
| PayLoad|   rate(%)    |PayLoad|    rate(%)    |
| Type(4)|    (8)       |Type(4)|       (8)     |
+--------+--------------+-------+---------------+

*/
class HeartBeat {
private:
    template<typename T1, typename T2, typename ...Args>
    static void _BuildPayLoad(std::vector<uint8_t>& data, T1 t1, T2 t2, Args... args);
    template<typename T1>
    static void _BuildPayLoad(std::vector<uint8_t>& data, T1 t1); // 处理单个参数的情况
    static void _BuildPayLoad(std::vector<uint8_t>& data); // 无参数的情况
    static void _AddHeader(std::vector<uint8_t>& header,uint8_t opType, uint16_t payloadLen, uint32_t sessionID);
public:
    template<typename ...Args>
    static std::string Build(uint8_t opType, uint32_t sessionID, Args... args);
private:
    const static int _payLoadUnitBits = 12;
    const static int _headerLen = 3;
    enum OpCode {PING, PONG};
    enum PayLoadType {CPU, MEM};

};

#endif