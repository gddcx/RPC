#include "tcp_send_buffer.h"

namespace crpc {

SendBuffer::SendBuffer(): pendingTaskNum(0),noBeginTaskNum(0) {}
SendBuffer::SendBuffer(const SendBuffer& buffer): pendingTaskNum(0),noBeginTaskNum(0) {
    _sendBuffer = buffer._sendBuffer;
}

std::vector<char> SendBuffer::GetBuffer() {
    std::vector<char> buffer = _sendBuffer.front();
    _sendBuffer.pop();
    return buffer;
}

void SendBuffer::AppendBuffer(std::vector<char>& buffer) {
    _sendBuffer.emplace(buffer);
    pendingTaskNum.fetch_add(1);
    noBeginTaskNum.fetch_add(1);
}

int SendBuffer::GetBufferSize() {
    return _sendBuffer.size();
}

void SendBuffer::Lock() {
    _bufMutex.lock();
}

void SendBuffer::Unlock() {
    _bufMutex.unlock();
}

}