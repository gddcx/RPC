#include <fcntl.h>
#include <sys/epoll.h>
#include <iostream>

#include "net_utils.h"

void SetNonBlockSock(uint32 fd) {
    int stat = fcntl(fd, F_GETFL, 0);
    stat |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, stat) == 0) {
        std::cout << "set fd:" << fd << " non block" << std::endl;
    }
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