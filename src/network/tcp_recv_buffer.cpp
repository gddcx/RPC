#include <iostream>
#include <algorithm>
#include "tcp_recv_buffer.h"

#define WRITE_CHECK_POS_GAP 1

namespace crpc{

RecvBuffer::RecvBuffer(): _bufferSize(4096+WRITE_CHECK_POS_GAP), _recvBuffer(4096+WRITE_CHECK_POS_GAP, 0) {}
RecvBuffer::RecvBuffer(int bufferSize): _bufferSize(bufferSize+WRITE_CHECK_POS_GAP), _recvBuffer(bufferSize+WRITE_CHECK_POS_GAP, 0) {}

RecvBuffer::RecvBuffer(const RecvBuffer& buffer) {
    _bufferSize = buffer._bufferSize;
    _recvBuffer = buffer._recvBuffer;
    _writePos = buffer._writePos;
    _checkpoint = buffer._checkpoint;
}

bool RecvBuffer::GetBuffer(int requireLen, std::vector<char>& out) {
    // _writePos不能追上checkpoint，但是checkpoint可以追上_writePos
    std::lock_guard<std::mutex> lock(_bufMutex);
    if(_writePos < _checkpoint) {
        if((_bufferSize - _checkpoint + _writePos) < requireLen) {
            return false;
        }
        int byteNum = std::min(requireLen, _bufferSize - _checkpoint);
        out.assign(_recvBuffer.begin() + _checkpoint, _recvBuffer.begin() + _checkpoint + byteNum);
        if(byteNum < requireLen) {
            out.insert(out.end(), _recvBuffer.begin(), _recvBuffer.begin() + requireLen - byteNum);
        }
    } else {
        if(_writePos - _checkpoint < requireLen) {
            return false;
        }
        out.assign(_recvBuffer.begin() + _checkpoint, _recvBuffer.begin() + _checkpoint + requireLen);
    }
    _checkpoint = (_checkpoint + requireLen) % _bufferSize;
    return true;
}

uint32 RecvBuffer::AppendBuffer(std::vector<char>& data, int offset, int len) {
    // _writePos == _checkpoint的时候有两种情况，一种是写满了，一种是空。没法判断这两种情况，所以写的时候要处理, 要gap一个byte
    for(auto c: data){
        std::cout << c;
    }
    std::cout << std::endl;
    std::lock_guard<std::mutex> lock(_bufMutex);
    uint32 appendByteNum = 0;
    int writableSize = 0;
    int validLen = 0;
    auto dataBegin = data.begin() + offset;

    if(_writePos < _checkpoint) {
        writableSize = _checkpoint - _writePos - WRITE_CHECK_POS_GAP;
    } else if(_writePos > _checkpoint) {
        writableSize = _bufferSize - _writePos + _checkpoint - WRITE_CHECK_POS_GAP;
    } else {
        writableSize = _bufferSize - WRITE_CHECK_POS_GAP;
    }

    validLen = std::min(writableSize, len);
    if(_writePos < _checkpoint) {
        _recvBuffer.insert(_recvBuffer.begin() + _writePos, dataBegin, dataBegin + validLen);
        appendByteNum = validLen;
    } else {
        if(_checkpoint == 0) {
            std::copy(dataBegin, dataBegin + validLen, _recvBuffer.begin() + _writePos);
            appendByteNum = validLen;
        } else {
            validLen = std::min(_bufferSize - _writePos, len);
            std::copy(dataBegin, dataBegin + validLen, _recvBuffer.begin() + _writePos);
            appendByteNum += validLen;
            dataBegin = dataBegin + validLen;
            validLen = std::min(writableSize, len) - validLen;
            std::copy(dataBegin, dataBegin + validLen, _recvBuffer.begin());
            appendByteNum += validLen;
        }
    }

    _writePos = (_writePos + appendByteNum) % _bufferSize;
    return appendByteNum;
}

void RecvBuffer::ClearBuffer() {
    _recvBuffer.clear();
    _writePos = 0;
    _checkpoint = 0;
}

}