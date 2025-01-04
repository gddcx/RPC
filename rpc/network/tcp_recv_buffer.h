#ifndef _TCP_RECV_BUFFER_
#define _TCP_RECV_BUFFER_

#include <vector>
#include <mutex>

#include "type.h"

namespace crpc {

class RecvBuffer
{
public:
    RecvBuffer();
    RecvBuffer(int bufferSize);
    RecvBuffer(const RecvBuffer& buffer);
    RecvBuffer& operator=(const RecvBuffer& buffer) {
        _bufferSize = buffer._bufferSize;
        _recvBuffer = buffer._recvBuffer;
        _writePos = buffer._writePos;
        _checkpoint = buffer._checkpoint;
        return *this;
    }

    bool GetBuffer(int requireLen, std::vector<char>& out);
    uint32 AppendBuffer(std::vector<char>& buffer, int offset, int len);
    void ClearBuffer();
    int GetSize() {
        return _recvBuffer.size();
    }
private:
    std::mutex _bufMutex;
    int _bufferSize;
    std::vector<char> _recvBuffer;
    int _writePos = 0;
    int _checkpoint = 0;
};
}

#endif