#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <string>
#include <unordered_map>

#include "type.h"

namespace crpc {
using namespace std;

class TcpClient
{
private:
    unordered_map<uint32, uint32> _connTbl;
    uint32 _epoll;
public:
    ~TcpClient();
    void InitClient();
    uint32 Connect(uint32 connName, string ipAddr, uint32 port);
};

}

#endif