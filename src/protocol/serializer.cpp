#if 0
#include <type_traits>
#include <vector>
#include "serializer.h"

// 直接按最长数据做zigzag，不然UnZigzag时比较麻烦
uint64_t Serializer::_Zigzag(uint64_t data) {
    return (data << 1) ^ (data >> 63);
}

uint64_t Serializer::_UnZigzag(uint64_t data) {
    return (data >> 1) ^ (-(data & 0x1));
}

template<typename T>
bool Serializer::_isVarintNum(T& oneData) {
    if constexpr(
        std::is_same<T, uint64_t>::value
        || std::is_same<T, int64_t>::value
        || std::is_same<T, uint32_t>::value
        || std::is_same<T, int32_t>::value
        || std::is_same<T, uint16_t>::value
        || std::is_same<T, int16_t>::value
        || std::is_same<T, uint8_t>::value
        || std::is_same<T, int8_t>::value
        || std::is_same<T, u_char>::value
        || std::is_same<T, char>::value) {
        return true;
    }
    else {
        return false;
    }
}

void Serializer::_VarintEncode(uint64_t data, std::string& res) {
    while(data >= 0x80) {
        res.push_back((uint8_t)data | 0x80);
        data = data >> 7;
    }
    res.push_back((uint8_t)data);
}

template<typename T>
void Serializer::_VarintDecode(uint8_t* data, T& res) {
    uint64_t num = 0;
    uint32_t bitOffset = 0;

    while(true) {
        num = num | ((*data & 0x7f) << bitOffset);
        bitOffset += 7;
        if((*data++ & 0x80) != 1) {
            break;
        }
    }

    res = (T)_UnZigzag(num);
}

template<typename T>
int Serializer::SerializeOne(int fieldNum, T& data, std::string& res) {
    if(_isVarintNum(data)) {
        res.push_back(fieldNum << 3 | VARINT);
        _VarintEncode(_Zigzag(data), res);
    }
}


void Serializer::DeSerializeOne(uint8_t* data, MessageBase& res) {
    WriteType writeByte = (WriteType)(*data & 0x7);
    int fieldNum = *data >> 3;
    std::string fieldName;
    res.GetFieldNameByFieldNum(fieldNum, fieldName);
    switch(writeByte) {
        case VARINT: {
            // _VarintDecode(data, res);
        }
    }
}
#endif