#include "http.h"

void Http::ParseHeader(const std::vector<char>& data) {
    bodyLen = 0;
    for(int offset = 0, bitOffset = 0; offset < _headerLen; offset++, bitOffset++) {
        bodyLen |= (int)(data[offset]) << (bitOffset * 8);
    }
}

int Http::GetHeaderLen() {
    return _headerLen;
}

int Http::SkipParseHttpHeader(int parseOffset) {
    return parseOffset + _headerLen;
}

std::string Http::GetHeader(short len) {
    std::vector<uint8_t> data(sizeof(len), 0);
    for(size_t i = 0; i < sizeof(len); i++) {
        data[i] = (uint8_t)((len >> (i * 8)) & 0xff);
    }
    return std::string(data.begin(), data.end());
}
