#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "tcp_server.h"
#include "net_utils.h"

namespace crpc {

TcpServer::~TcpServer() {
    CloseServer();
}

void TcpServer::_ProcessMessage() {
    Message msg;
    {
        std::unique_lock<std::mutex> lock(_msgQueueMutex);
        _msgCV.wait(lock, [&](){return !_messageQueue.empty() || !_running;}); // 消息队列不为空 或 Server被关闭

        if(!_running && _messageQueue.empty()) { // Server被关闭 且 这个线程没有任务要处理
            return;
        }

        msg = _messageQueue.front();
        _messageQueue.pop();
    }
    if(_onMessage) {
        _onMessage(msg._clientFd, msg._message);
    } else {
        std::cout << __func__ << ":No definition of _onMessage function" << std::endl;
    }
    
}

void TcpServer::_InitThreadPool(uint32 threadNum) {
    std::cout << __func__ << ": create thread poll. thread num=" << threadNum << std::endl;
    for(uint32 n = 0; n < threadNum; n++) {
        _threadPool.emplace_back(&TcpServer::_ProcessMessage, this);
    }
}

int TcpServer::_InitServerSocket(uint32 port) {
    _epollFd = epoll_create1(EPOLL_CLOEXEC);

    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_serverFd == -1) {
        std::cout << __func__ << ":create server socket fail" << std::endl;
        return -1;
    }

    sockaddr_in sockInfo;
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    sockInfo.sin_port = htons(port);
    bind(_serverFd, (sockaddr*)&sockInfo, sizeof(sockaddr_in));

    SetNonBlockSock(_serverFd);
    EpollAddSock(_epollFd, _serverFd, EPOLLIN);

    if(listen(_serverFd, SOMAXCONN) == 0) {
        std::cout << __func__ << ":start listening port:" << port << std::endl;
        return 0;
    }

    return -1;
}

uint32 TcpServer::InitServer(uint32 port, uint32 threadNum) {
    _InitThreadPool(threadNum);
    return _InitServerSocket(port);
}

void TcpServer::_AcceptNewConn() {
    sockaddr_in sockInfo;
    uint32 sockInfoLen = sizeof(sockaddr_in);

    memset(&sockInfo, 0, sizeof(sockaddr));
    int clientFd = accept(_serverFd, (sockaddr*)&sockInfo, &sockInfoLen);
    if(clientFd == -1) {
        std::cout << __func__ << ":accept new connection fail. errno:" << errno << std::endl;
    }
    else {
        SetNonBlockSock(clientFd);
        EpollAddSock(_epollFd, clientFd, EPOLLIN | EPOLLET);
        std::cout << __func__ << ":accept new connection succ. peer ip:" << ntohl(sockInfo.sin_addr.s_addr) << "peer port:" << ntohs(sockInfo.sin_port) << std::endl;
        {
            std::lock_guard<std::mutex> lock(_clientFdMutex);
            _clientFds.emplace(clientFd, 0);
        }
        if(_onConnect) {
            _onConnect(clientFd);
        }
    }
}

uint32 TcpServer::_ReadSocket(uint32 fd) {
    uint32 totalBytes = 0;
    uint32 byteNum = 0;
    char recvBuffer[4096];

    while(totalBytes < 4096) {
        byteNum = recv(fd, recvBuffer+totalBytes, 4096-totalBytes, MSG_DONTWAIT);
        if(byteNum < 0) {
            switch (errno) {
                case EAGAIN:  // 连接正常但是没有更多数据可以读了
                    break;
                case EINTR:   // recv被中断
                    continue;
                default:      // 其它异常
                    std::cout << "recv err. errno:" << errno << std::endl;
                    if(_onClose) _onClose(fd);
                    close(fd);
                    EpollDelSock(_epollFd, fd);
                    return -1;
            }
        }
        else if (byteNum == 0) { // 对端关闭了连接
            std::cout << "peer close connection." << std::endl;
            if(_onClose) _onClose(fd);
            close(fd);
            EpollDelSock(_epollFd, fd);
            return 0;
        }
        else {
            totalBytes += byteNum;
        }
    }

    {
        std::lock_guard<std::mutex> lock(_msgQueueMutex);
        _messageQueue.emplace(fd, std::string(recvBuffer, totalBytes));
    }

    return totalBytes;
}

uint32 TcpServer::_WriteSocket(uint32 fd) {
    return 0;
}

void TcpServer::_EventHandle(uint32 fd, uint32 event) {
    switch(event) {
        case EPOLLIN:
            _ReadSocket(fd);
            break;
        case EPOLLOUT:
            _WriteSocket(fd);
            break;
        default:
            break;
    }
}

void TcpServer::StartServer() {
    std::cout << __func__ << ":start listening I/O events." << std::endl;

    epoll_event ev[512];
    uint32 maxEvLen = sizeof(ev) / sizeof(ev[0]);
    uint32 eventNum = 0;

    _running = true;
    for(;;)
    {
        if(!_running) break;
        eventNum = epoll_wait(_epollFd, &ev[0], maxEvLen, -1);
        for(uint32 idx = 0; idx < eventNum; idx++) {
            if(ev[idx].data.fd == _serverFd) {    // 新连接
                _AcceptNewConn(); // server应该要管理这里打开的文件描述符，不能依赖上层的事件处理函数
            }
            else {    // I/O读写事件
                _EventHandle(ev[idx].data.fd, ev[idx].events);
            }
        }
    }
}

void TcpServer::CloseServer() {
    _running = false;
    _msgCV.notify_all();
    for(auto& thread: _threadPool) { // 等待还有任务的线程执行完。这里无差别join任务/空闲线程，所以要notify_all把空闲线程也唤醒
        if(thread.joinable()) thread.join();
    }

    std::lock_guard<std::mutex> lock(_clientFdMutex);
    for(auto& pair: _clientFds) {
        if(_onClose) _onClose(pair.first);
        close(pair.first);
    }
    if(_serverFd != -1) close(_serverFd);
    if(_epollFd != -1) close(_epollFd);
}

void TcpServer::SetOnMessage(const OnMessageCallback& onMessage) {
    _onMessage = onMessage;
}

void TcpServer::SetOnConnect(const OnConnectCallback& onConnect) {
    _onConnect = onConnect;
}

void TcpServer::SetOnClose(const OnCloseCallback& onClose) {
    _onClose = onClose;
}

} // namespace crpc