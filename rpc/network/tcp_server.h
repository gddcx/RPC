#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include <string>
#include <iostream>

#include "type.h"
#include "tcp_recv_buffer.h"
#include "tcp_send_buffer.h"

namespace crpc {

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

using OnMessageCallback = std::function<void(int clientFd, RecvBuffer& buffer)>;
using OnConnectCallback = std::function<void(int clientFd)>;
using OnCloseCallback = std::function<void(int clientFd)>;

class TcpServer {
private:
    uint32 _ReadSocket(uint32 fd);
    uint32 _WriteSocket(uint32 fd);
    void _ProcessMessage();
    void _InitThreadPool(uint32 threadNum);
    int _InitServerSocket(uint32 port);
    void _AcceptNewConn();
    void _EventHandle(uint32 fd, uint32 event);
public:
    ~TcpServer();
    uint32 InitServer(uint32 port, uint32 threadNum, uint32 recvBufferSize);
    void StartServer();
    void CloseServer();
    void SendMsg(int fd, std::vector<char>& sendBuffer);
    void Disconnection(int fd);
    void SetOnMessage(const OnMessageCallback& onMessage);
    void SetOnConnect(const OnConnectCallback& onConnect);
    void SetOnClose(const OnCloseCallback& onClose);

private:
    int _epollFd = -1;
    int _serverFd = -1;
    bool _running = true;
    uint32 _recvBufferSize = 0;
    std::mutex _taskQueMutex;
    std::condition_variable _taskCV;
    std::queue<Message> _taskQueue;
    std::vector<std::thread> _threadPool;
    std::mutex _channelMutex;
    std::unordered_map<int, TcpChannel> _channels;
    OnMessageCallback _onMessage = nullptr;
    OnConnectCallback _onConnect = nullptr;
    OnCloseCallback _onClose = nullptr;
};

}

#endif