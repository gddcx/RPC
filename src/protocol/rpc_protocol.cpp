#include <algorithm>
#include "rpc_protocol.h"

void RpcProtocol::_AddMsgHeader(int requestId, char msgType, std::string& para) {
    para = Msg::GetHeader(requestId, msgType) + para;
}

void RpcProtocol::_AddHttpHeader(short bodyLen, std::string& para) {
    para = Http::GetHeader(bodyLen) + para;
}

void RpcProtocol::_ParseHttp(const std::vector<char>& data) {
    _http.ParseHeader(data);
}

void RpcProtocol::_ParseMsg(const std::vector<char>& data) {
    _msg.ParseMsg(data);
}

void RpcProtocol::Build(int requestId, char msgType, std::string& para) {
    _AddMsgHeader(requestId, msgType, para);
    _AddHttpHeader(para.size(), para);
}

int RpcProtocol::GetHttpHeaderLen() {
    return _http.GetHeaderLen();
}

void RpcProtocol::ParseHttp(const std::vector<char>& data) {
    _ParseHttp(data);
}

int RpcProtocol::GetMsgLen() {
    return _http.bodyLen;
}

void RpcProtocol::ParseMsg(const std::vector<char>& data) {
    _ParseMsg(data);
}

int RpcProtocol::GetRequestId() {
    return _msg.requestId;
}

uint8_t RpcProtocol::GetMsgType() {
    return _msg.msgType;
}

std::string& RpcProtocol::GetMsg() {
    return _msg.msg;
}