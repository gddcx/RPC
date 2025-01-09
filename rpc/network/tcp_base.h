#ifndef _TCP_BASE_H_
#define _TCP_BASE_H_

#include <functional>
#include <unordered_map>
#include <condition_variable>
#include "tcp_recv_buffer.h"
#include "tcp_send_buffer.h"

namespace crpc
{

using OnMessageCallback = std::function<void(int clientFd, RecvBuffer& buffer)>;
using OnConnectCallback = std::function<void(int clientFd)>;
using OnCloseCallback = std::function<void(int clientFd)>;

struct TcpChannel {
    int _clientFd;
    SendBuffer _sendBuffer;
    RecvBuffer _recvBuffer;
    bool closeFlag = false;

    TcpChannel() {}
    TcpChannel(int fd, int recvBufferSize): _clientFd(fd), _recvBuffer(recvBufferSize) {}
    TcpChannel(const TcpChannel& channel) {
        _clientFd = channel._clientFd;
        _sendBuffer = channel._sendBuffer;
        _recvBuffer = channel._recvBuffer;
    }
    TcpChannel& operator=(const TcpChannel& channel) {
        _clientFd = channel._clientFd;
        _sendBuffer = channel._sendBuffer;
        _recvBuffer = channel._recvBuffer;
        return *this;
    }
};

enum MsgType {
    Writable,
    Readable
};

struct Message {
    int _fd;
    MsgType _msgType;
    Message() {}
    Message(int fd, MsgType msgType): _fd(fd), _msgType(msgType) {}
};

using threadFunc = std::function<void(int)>;
struct ThreadPara {
    std::condition_variable cv;
    std::queue<Message> taskQueue;
    std::mutex taskQueMutex;
    std::thread t;
};

class TcpBase {
protected:
    int _WriteSocket(int fd);
    int _ReadSocket(int fd);
    void _ProcessMessage(int threadIndex);
    void _InitThreadPool();
    void _CommonInit(int recvBufferSize);
    void _EventHandle(uint32 fd, uint32 event);
    void _CommClose();
public:
    TcpBase(int threadNum);
    void SendMsg(int fd, std::vector<char>& sendBuffer);
    void SetOnMessage(const OnMessageCallback& onMessage);
    void SetOnConnect(const OnConnectCallback& onConnect);
    void SetOnClose(const OnCloseCallback& onClose);
protected:
    int _epollFd;
    int _eventFd;
    int _recvBufferSize = 0;
    int _threadNum;
    bool _running = true;
    std::thread _mainThread;
    std::vector<ThreadPara> _threadPool;
    std::mutex _channelMutex;
    std::unordered_map<int, TcpChannel> _channels;
    OnMessageCallback _onMessage = nullptr;
    OnConnectCallback _onConnect = nullptr;
    OnCloseCallback _onClose = nullptr;
};

}

#endif