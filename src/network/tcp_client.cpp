#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sys/eventfd.h>
#include <fcntl.h>
#include "tcp_client.h"
#include "net_utils.h"

namespace crpc{

TcpClient::TcpClient(int threadNum): TcpBase(threadNum) {}

TcpClient::~TcpClient() {
    CloseClient();
}

void TcpClient::InitClient(int recvBufferSize = 4096) {
    _CommonInit(recvBufferSize);
    _eventFd = eventfd(0, FD_CLOEXEC | EFD_NONBLOCK);
    fcntl(_eventFd, F_SETFL, FD_CLOEXEC);
    EpollAddSock(_epollFd, _eventFd, EPOLLIN | EPOLLET);
}

void TcpClient::_StartClient() {
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
            if (fcntl(_epollFd, F_GETFD) == -1) {
                std::cout << "Invalid _epollFd" << std::endl;
            }
            std::cout << "epoll_wait error:" << errno << std::endl;
            continue;
        }
        for(idx = 0; idx < eventNum; idx++) {
            if (ev[idx].data.fd != _eventFd) { // I/O读写事件
                _EventHandle(ev[idx].data.fd, ev[idx].events);
            } else {
                continue;
            }
        }
    }
}

void TcpClient::StartClient() {
    _mainThread = std::thread(&TcpClient::_StartClient, this);
}

int TcpClient::Connect(std::string ipAddr, uint16 port) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd < 0) {
        std::cout << __func__ << ">>> " << "create socket fail. errno:" << errno << std::endl;
        return -1;
    }

    sockaddr_in sockInfo;
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    sockInfo.sin_port = htons(port);
    if(connect(fd, (sockaddr*)&sockInfo, sizeof(sockaddr)) == 0)
    {
        std::cout << __func__ << ">>> " << "connect to " << ipAddr << ":" << port << " succ. fd:" << fd << std::endl;
        SetNonBlockSock(fd); // TODO:放在这个位置设置block的话，如果connect不上就会一阻塞
        SetNoDelay(fd);
        EpollAddSock(_epollFd, fd, EPOLLIN | EPOLLOUT | EPOLLET);
        std::lock_guard<std::mutex> channLock(_channelMutex);
        _channels.emplace(fd, TcpChannel(fd, _recvBufferSize));
        return fd;
    }
    std::cout << __func__ << ">>> " << "connect to " << ipAddr << ":" << port << " fail. errno:" << errno << std::endl;
    close(fd);
    return -1;
}

void TcpClient::CloseClient() {
    _running = false;
    uint64_t flag = 1;
    if(_eventFd != -1) write(_eventFd, &flag, sizeof(flag));
    if(_mainThread.joinable()) _mainThread.join();
    if(_eventFd != -1) close(_eventFd);
}

}