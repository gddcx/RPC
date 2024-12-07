#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include "tcp_client.h"
#include "net_utils.h"

namespace crpc{
TcpClient::~TcpClient() {
    for(auto conn: _connTbl) {
        close(conn.second);
    }
    close(_epoll);
}

void TcpClient::InitClient() {
    _epoll = epoll_create1(EPOLL_CLOEXEC);
}

uint32 TcpClient::Connect(uint32 connName, string ipAddr, uint32 port) {
    if(_connTbl.find(connName) != _connTbl.end())
    {
        close(_connTbl[connName]);
    }

    uint32 fd = socket(AF_INET, SOCK_STREAM, 0);
    SetNonBlockSock(fd);
    EpollAddSock(_epoll, fd, EPOLLIN | EPOLLET);

    sockaddr_in sockInfo;
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    sockInfo.sin_port = htons(port);
    if(connect(fd, (sockaddr*)&sockInfo, sizeof(sockaddr_in)) == 0)
    {
        std::cout << "connect to " << ipAddr << ":" << port << " succ." << std::endl;
        _connTbl.emplace(connName, fd);
        return 0;
    }
    std::cout << "connect to " << ipAddr << ":" << port << " fail. errno:" << errno << std::endl;
    return -1;
}
}