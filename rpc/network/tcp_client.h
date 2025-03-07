#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <thread>
#include <utility>

#include "type.h"
#include "tcp_base.h"

namespace crpc {

class TcpClient: public TcpBase{
private: 
    void _StartClient();
public:
    TcpClient(int threadNum);
    ~TcpClient();
    void InitClient(int recvBufferSize);
    void StartClient();
    void CloseClient();
    int Connect(std::string ipAddr, uint16 port);
private:
    int _eventFd = -1;
    std::thread _mainThread;
};

}

#endif