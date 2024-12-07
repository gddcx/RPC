#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include "type.h"

namespace crpc {

struct Message {
    int _clientFd;
    std::string _message;
    Message() {}
    Message(int clientFd, std::string message): _clientFd(clientFd), _message(message) {}
};

using OnMessageCallback = std::function<void(int clientFd, std::string& message)>;
using OnConnectCallback = std::function<void(int clientFd)>;
using OnCloseCallback = std::function<void(int clientFd)>;

class TcpServer {
private:
    void _ProcessMessage();
    void _InitThreadPool(uint32 threadNum);
    int _InitServerSocket(uint32 port);
    void _AcceptNewConn();
    uint32 _ReadSocket(uint32 fd);
    uint32 _WriteSocket(uint32 fd);
    void _EventHandle(uint32 fd, uint32 event);
public:
    ~TcpServer();
    uint32 InitServer(uint32 port, uint32 threadNum);
    void StartServer();
    void CloseServer();
    void SetOnMessage(const OnMessageCallback& onMessage);
    void SetOnConnect(const OnConnectCallback& onConnect);
    void SetOnClose(const OnCloseCallback& onClose);

private:
    int _epollFd = -1;
    int _serverFd = -1;
    bool _running = false;
    std::mutex _msgQueueMutex;
    std::mutex _clientFdMutex;
    std::condition_variable _msgCV;
    std::queue<Message> _messageQueue;
    std::vector<std::thread> _threadPool;
    std::unordered_map<int, int> _clientFds;
    OnMessageCallback _onMessage = nullptr;
    OnConnectCallback _onConnect = nullptr;
    OnCloseCallback _onClose = nullptr;
};

}

#endif