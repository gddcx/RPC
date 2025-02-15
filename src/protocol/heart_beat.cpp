#include <iostream>
#include "heart_beat.h"

void HeartBeat::_BuildPayLoad(std::vector<uint8_t>& data) {
}

template<typename T1>
void HeartBeat::_BuildPayLoad(std::vector<uint8_t>& data, T1 t1) {
    std::cout << __func__ << ": para must be pairs." << std::endl;
}

template<typename T1, typename T2, typename ...Args>
void HeartBeat::_BuildPayLoad(std::vector<uint8_t>& data, T1 t1, T2 t2, Args... args) {
    _BuildPayLoad(data, args...);
    int remaingUnitnum = sizeof...(args) / 2;
    int bitOffset = (remaingUnitnum * _payLoadUnitBits) % 8;
    int byteOffset = (remaingUnitnum * _payLoadUnitBits) / 8;
    uint8_t* ptr = data.data();
    if(bitOffset == 0) {
        *(ptr + byteOffset) = (((t2 & 0xf) << 4) | (t1 & 0xf));
        *(ptr + byteOffset + 1) |= ((t2 >> 4) & 0xf);
    } else {
        *(ptr + byteOffset) |= ((t1 & 0xf) << bitOffset);
        *(ptr + byteOffset + 1) = (t2 & 0xff);
    }
}

void HeartBeat::_AddHeader(std::vector<uint8_t>& header, uint8_t opType, uint16_t payloadLen, uint32_t sessionID) {
    header[0] = opType & 0x1;
    header[0] |= (payloadLen & 0x7f) << 1;
    header[1] = sessionID & 0xff;
    header[2] = (sessionID >> 8) & 0xff;
}

template<typename ...Args>
std::string HeartBeat::Build(uint8_t opType, uint32_t sessionID, Args... args) {
    int payloadUnitNum = sizeof...(args) / 2;
    int payLoadByte = (float)(payloadUnitNum * _payLoadUnitBits) / 8  + 0.5;
    std::vector<uint8_t> payLoad(payLoadByte, 0);
    std::vector<uint8_t> header(_headerLen, 0);

    _BuildPayLoad(payLoad, args...);
    _AddHeader(header, opType, payLoadByte, sessionID);

    std::string frameData = std::string(header.begin(), header.end()) + std::string(payLoad.begin(), payLoad.end());
    return frameData;
}