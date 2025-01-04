#ifndef _TCP_SEND_BUFFER_
#define _TCP_SEND_BUFFER_

#include <queue>
#include <vector>
#include <mutex>
#include <atomic>

namespace crpc {

class SendBuffer
{
public:
    SendBuffer();
    SendBuffer(const SendBuffer& buffer);
    std::vector<char> GetBuffer();
    void AppendBuffer(std::vector<char>& buffer);
    int GetBufferSize(); // 还没开始写的数据条数(未写)
    void Lock();
    void Unlock();
    SendBuffer& operator=(const SendBuffer& buffer) {
        _sendBuffer = buffer._sendBuffer;
        return *this;
    }
public:
    std::atomic<int> pendingTaskNum; // 未完成写的数据条数(未写、正在写)
private:
    std::queue<std::vector<char>> _sendBuffer;
    std::mutex _bufMutex;
};

}

#endif