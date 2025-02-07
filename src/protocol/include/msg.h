#ifndef _MSG_H_
#define _MSG_H_

#include <unordered_map>
#include <string>

/*
    ------------------------------------------
    | Request ID 4byte | MsgType 1byte | Msg |
    ------------------------------------------
*/

class Msg {
private:
    void _ParseHeader(const std::vector<char>& data);
public:
    void ParseMsg(const std::vector<char>& data);
    static std::string GetHeader(int requestId, char msgType);
private:
    static const int _requestIdLen = 4;
    static const int _msgTypeLen = 1;
public:
    int requestId;
    uint8_t msgType;
    std::string msg;
};

#endif