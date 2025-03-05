#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <utility>
#include <atomic>
#include <sys/eventfd.h>
#include <fcntl.h>
#include "tcp_server.h"
#include "net_utils.h"

namespace crpc {

TcpServer::TcpServer(int threadNum): TcpBase(threadNum) {}

TcpServer::~TcpServer() {
    CloseServer();
}

int TcpServer::_InitServerSocket(uint32 port) {
    _serverFd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(_serverFd == -1) {
        std::cout << __func__ << ">>> " << "create server socket fail" << std::endl;
        return -1;
    }

    sockaddr_in sockInfo;
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    sockInfo.sin_port = htons(port);
    bind(_serverFd, (sockaddr*)&sockInfo, sizeof(sockaddr_in));

    SetNonBlockSock(_serverFd);
    SetCloexecSock(_serverFd);
    EpollAddSock(_epollFd, _serverFd, EPOLLET | EPOLLIN);

    if(listen(_serverFd, SOMAXCONN) == 0) {
        std::cout << __func__ << ">>> " << "start listening port:" << port << std::endl;
        return 0;
    }

    std::cout << __func__ << ">>> " << "start server fail" << port << std::endl;
    return -1;
}

int TcpServer::InitServer(uint32 port, int recvBufferSize = 4096) {
    _CommonInit(recvBufferSize);
    _eventFd = eventfd(0, FD_CLOEXEC | EFD_NONBLOCK);
    fcntl(_eventFd, F_SETFL, FD_CLOEXEC);
    EpollAddSock(_epollFd, _eventFd, EPOLLIN | EPOLLET);
    return _InitServerSocket(port);
}

void TcpServer::_AcceptNewConn() {
    sockaddr_in sockInfo = {0};
    uint32 sockInfoLen = sizeof(sockaddr_in);

    while(true) {
        int clientFd = accept(_serverFd, (sockaddr*)&sockInfo, &sockInfoLen);
        if(clientFd == -1) {
            std::cout << __func__ << ">>> " << "accept new connection fail. errno:" << errno << std::endl;
            if(errno == EAGAIN) {
                break;
            } else { // TODO:其它异常待补充
                break;
            }
        } else {
            SetNonBlockSock(clientFd);
            SetCloexecSock(clientFd);
            SetNoDelay(clientFd);
            EpollAddSock(_epollFd, clientFd, EPOLLIN | EPOLLOUT | EPOLLET);
            std::cout << __func__ << ">>> " << "accept new connection succ. peer ip:" << ntohl(sockInfo.sin_addr.s_addr) << ", peer port:" << ntohs(sockInfo.sin_port) << " fd:" << clientFd << std::endl;
            {
                std::lock_guard<std::mutex> lock(_channelMutex); // TODO: 做定时器关闭连接
                _channels.emplace(clientFd, TcpChannel(clientFd, _recvBufferSize)); // 有三次构造，临时对象生成A，_channel内调用TcpChannel默认构造生成B，A拷贝构造覆盖B
            }
            if(_onConnect) {
                _onConnect(clientFd);
            }
        }
    }
}

void TcpServer::_StartServer() {
    std::cout << __func__ << ">>> " << "start listening I/O events." << std::endl;

    epoll_event ev[512];
    uint32 maxEvLen = sizeof(ev) / sizeof(ev[0]);
    int eventNum = 0;
    int idx = 0;

    _running = true;
    while(true)
    {
        if(!_running) break;
        eventNum = epoll_wait(_epollFd, &ev[0], maxEvLen, -1);
        if(eventNum == -1) {
            std::cout << "epoll_wait error:" << errno << std::endl;

            continue;
        }
        for(idx = 0; idx < eventNum; idx++) {
            if(ev[idx].data.fd == _serverFd) {    // 新连接
                _AcceptNewConn();
            } else if (ev[idx].data.fd == _eventFd) {
                continue;
            } else {    // I/O读写事件
                _EventHandle(ev[idx].data.fd, ev[idx].events);
            }
        }
    }
}

void TcpServer::StartServer() {
    _mainThread = std::thread(&TcpServer::_StartServer, this);
}

void TcpServer::CloseServer() {
    _running = false;
    uint64_t flag = 1;
    if(_eventFd != -1) write(_eventFd, &flag, sizeof(flag));
    if(_mainThread.joinable()) _mainThread.join();
    if(_serverFd != -1) close(_serverFd);
}

} // namespace crpc