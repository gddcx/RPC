#include <vector>
#include <iostream>
#include "msg.h"

void Msg::_ParseHeader(const std::vector<char>& data) {
    int offset = 0;
    int tmpData = 0;
    int bitOffset = 0;
    requestId = 0;

    while(offset < _requestIdLen) {
        tmpData = data[offset];
        requestId |= tmpData << (bitOffset * 8);
        offset++;
        bitOffset++;
    }

    msgType = (uint8_t)data[offset];
}

void Msg::ParseMsg(const std::vector<char>& data) {
    _ParseHeader(data);
    int headerLen = _requestIdLen + _msgTypeLen;
    msg = std::string(data.begin() + headerLen, data.end());
}


std::string Msg::GetHeader(int requestId, char msgType) {
    std::vector<uint8_t> data(_requestIdLen + _msgTypeLen, 0);
    int offset = 0;

    while(offset < _requestIdLen) {
        data[offset] = (uint8_t)((requestId >> (offset * 8)) & 0xff);
        offset++;
    }

    data[offset] = msgType;
    return std::string(data.begin(), data.end());
}