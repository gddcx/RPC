#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include "tcp_client.h"
#include "net_utils.h"

namespace crpc{

TcpClient::TcpClient(int threadNum): TcpBase(threadNum) {}

TcpClient::~TcpClient() {
    CloseClient();
}

void TcpClient::InitClient(int recvBufferSize = 4096) {
    _CommonInit(recvBufferSize);
}

void TcpClient::_StartClient() {
    std::cout << __func__ << ">>> " << "start listening I/O events." << std::endl;

    epoll_event ev[512];
    uint32 maxEvLen = sizeof(ev) / sizeof(ev[0]);
    uint32 eventNum = 0;

    _running = true;
    while(true)
    {
        if(!_running) break;
        eventNum = epoll_wait(_epollFd, &ev[0], maxEvLen, -1);
        for(uint32 idx = 0; idx < eventNum; idx++) {
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
    _mainThread.detach();
}

int TcpClient::Connect(std::string ipAddr, short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sockInfo;
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    sockInfo.sin_port = htons(port);
    if(connect(fd, (sockaddr*)&sockInfo, sizeof(sockaddr)) == 0)
    {
        std::cout << __func__ << ">>> " << "connect to " << ipAddr << ":" << port << " succ." << std::endl;
        SetNonBlockSock(fd);
        EpollAddSock(_epollFd, fd, EPOLLIN | EPOLLOUT | EPOLLOUT);
        std::lock_guard<std::mutex> channLock(_channelMutex);
        _channels.emplace(fd, TcpChannel(fd, _recvBufferSize));
        return fd;
    }
    std::cout << __func__ << ">>> " << "connect to " << ipAddr << ":" << port << " fail. errno:" << errno << std::endl;
    close(fd);
    return -1;
}

void TcpClient::CloseClient() {
    _CommClose();
}

}