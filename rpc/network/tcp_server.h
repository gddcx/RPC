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
#include "tcp_base.h"

namespace crpc {

class TcpServer: public TcpBase {
private:
    int _InitServerSocket(uint32 port);
    void _AcceptNewConn();
    void _StartServer();
public:
    TcpServer(int threadNum);
    ~TcpServer();
    int InitServer(uint32 port, int recvBufferSize);
    void StartServer();
    void CloseServer();

private:
    int _serverFd = -1;
};

}

#endif