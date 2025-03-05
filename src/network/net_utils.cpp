#include <fcntl.h>
#include <sys/epoll.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include "net_utils.h"

void SetNonBlockSock(uint32 fd) {
    int stat = fcntl(fd, F_GETFL, 0);
    stat |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, stat) == 0) {
        std::cout << __func__ << ">>> set fd'" << fd << " non block" << std::endl;
    }
}

void SetCloexecSock(uint32 fd) {
    int stat = fcntl(fd, F_GETFD, 0);
    stat |= FD_CLOEXEC;
    if(fcntl(fd, F_SETFD, stat) == 0) {
        std::cout << __func__ << ">>> set fd'" << fd << " cloexec" << std::endl;
    }
}

void SetNoDelay(int fd) {
    int flag = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

void EpollAddSock(uint32 epoll, uint32 fd, uint32 event) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;
    epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &ev);
}

void EpollDelSock(uint32 epoll, uint32 fd) {
    epoll_ctl(epoll, EPOLL_CTL_DEL, fd, nullptr);
}

void EpollModSock(uint32 epoll, uint32 fd, uint32 event) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;
    epoll_ctl(epoll, EPOLL_CTL_MOD, fd, &ev);
}