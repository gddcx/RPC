#ifndef _PROTOCOL_COMM_H_
#define _PROTOCOL_COMM_H_


#include <string>
#include <vector>

enum MessageType {
    PING,
    PONG,
    FUNC_REGISTER,
    FUNC_QUERY,
    RPC
};

/* 

 0               1               2
 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+---------------+-------------------------------+
| message Type  |              UUID             |
+---------------+----------------+--------------+
|           Body Length          |              |
+---------------+----------------+--------------+

*/

class ProtocolComm {
public:
    static std::string BulidComm(uint8_t type, uint16_t id, uint16_t len);
    void ParseComm(const std::vector<char>& data);
public: 
    static constexpr int commHeaderLen = 5;
    uint8_t protoMsgType;
    uint16_t protoUUID;
    uint16_t protoMsgLen;
};

#endif