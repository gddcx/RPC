#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#if 0
#include <cstdint>
#include <string>
// #include "message_base.h"

enum WriteType {
    VARINT,
    BIT64,
    LENGTH_DELIMITED,
    START_GROUP,
    END_GROUP,
    BIT32,
};

class Serializer {
private:
    uint64_t _Zigzag(uint64_t data);
    uint64_t _UnZigzag(uint64_t data);
    template<typename T>
    bool _isVarintNum(T& oneData);
    void _VarintEncode(uint64_t data, std::string& res);
    template<typename T>
    void _VarintDecode(uint8_t* oneData, T& res);
public:
    template<typename T>
    int SerializeOne(int fieldNum, T& oneData, std::string& res);
    void GetKey(uint8_t* data);
    void DeSerializeOne(uint8_t* data, MessageBase& res);
};
#endif
#endif