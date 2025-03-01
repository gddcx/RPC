#ifndef _MONITORING_H_
#define _MONITORING_H_

#include <unistd.h>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <vector>
#include <array>
#include "tcp_client.h"
#include "tcp_server.h"
#include "timer.h"
#include "ipc_msg.h"
#include "logging.h"

using namespace crpc;

class Monitoring {
private:
    void _onConnectCallback(int fd);
    void _onMessageCallback(int fd, RecvBuffer& recvBuf);
    void _onCloseCallback(int fd);
    void _SafeInsert(uint16_t uuid, std::function<void()> func);
    void _SafeErase(uint16_t uuid);
    void _SendHeartBeat();
    void _RecvMsg();
    void _KillRpcServer();
    void _StartRpcServer();
    void _Rescue();
public:
    Monitoring();
    ~Monitoring();
    void StartMonitoring();
private:
    TcpServer _tcpServer;
    TcpClient _tcpClient;
    int _fd = -1;
    std::atomic<uint16_t> _uuid;
    std::atomic<uint32_t> _missWithCenter;
    std::atomic<uint32_t> _clientPing;
    std::mutex _callbackMutex;
    std::unordered_map<uint32_t, std::function<void()>> _callbacks;
    Timer _timer;
    IPCMsgQueue _ipcMsgQueue;
    std::vector<uint8_t> _ipcMsg;
    enum IPCMsgType {INVALID, SCORE, TYPE_NUM};
    Logger _logger;
    pid_t _rpcPid;
};

#endif