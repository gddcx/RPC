#ifndef _RPC_PROTOCOL_H_
#define _RPC_PROTOCOL_H_

#include <vector>
#include "http.h"
#include "msg.h"

/*
    ----------------------------------
    | HTTP Header | Msg Header | Msg |
    ----------------------------------
*/

class RpcProtocol {
private:
    Http _http;
    Msg _msg;
private:
    static void _AddMsgHeader(int requestId, char msgType, std::string& para);
    static void _AddHttpHeader(short bodyLen, std::string& para);

    void _ParseHttp(const std::vector<char>& data);
    void _ParseMsg(const std::vector<char>& data);
public:
    static void Build(int requestId, char msgType, std::string& para);

    void ParseHttp(const std::vector<char>& data);
    void ParseMsg(const std::vector<char>& data);

    int GetHttpHeaderLen();
    int GetMsgLen();
    int GetRequestId();
    uint8_t GetMsgType();
    std::string& GetMsg();
};

#endif