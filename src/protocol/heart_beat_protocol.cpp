#include <iostream>
#include "heart_beat_protocol.h"

void HeartBeatProtocol::_BuildPayLoad(std::vector<uint8_t>& data, std::vector<uint8_t>& scores) {
    int bitOffset;
    int byteOffset;
    uint8_t* ptr = data.data();
    for(size_t typeIdx = 0; typeIdx < scores.size(); ++typeIdx) {
        bitOffset = (typeIdx * _payLoadUnitBits) % 8;
        byteOffset = (typeIdx * _payLoadUnitBits) / 8;
        if(bitOffset == 0) {
            *(ptr + byteOffset) = (((scores[typeIdx] & 0xf) << 4) | (typeIdx & 0xf));
            *(ptr + byteOffset + 1) |= ((scores[typeIdx] >> 4) & 0xf);
        } else {
            *(ptr + byteOffset) |= ((typeIdx & 0xf) << bitOffset);
            *(ptr + byteOffset + 1) = (scores[typeIdx] & 0xff);
        }
    }
}

std::string HeartBeatProtocol::Build(uint8_t msgType, uint32_t id, std::vector<uint8_t>& scores) {
    int payLoadByte = (float)(scores.size() * _payLoadUnitBits) / 8  + 0.5;
    std::vector<uint8_t> payLoad(payLoadByte, 0);

    _BuildPayLoad(payLoad, scores);
    std::string frameData = ProtocolComm::BulidComm(msgType, id, payLoadByte) + std::string(payLoad.begin(), payLoad.end());

    return frameData;
}

std::string HeartBeatProtocol::Build(uint8_t msgType, uint32_t id) {
    return ProtocolComm::BulidComm(msgType, id, 0);
}

void HeartBeatProtocol::ParseHeader(const std::vector<char>& data) {
    ParseComm(data);
}

void HeartBeatProtocol::ParseBody(const std::vector<char>& data) {
    int payLoadNum = data.size();
    bool isOdd = true;
    for(int idx = 0; idx < payLoadNum; idx++) {
        if(isOdd) {
            payLoad[(PayLoadType)(data[idx] & 0xf)] = ((data[idx+1] & 0xf) << 4) | ((data[idx] >> 4) & 0xf);
            isOdd = false;
        } else {
            payLoad[(PayLoadType)((data[idx] >> 4) & 0xf)] = data[idx+1];
            idx++;
            isOdd = true;
        }
    }
}