#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <csignal>
#include "monitoring.h"
#include "heart_beat_protocol.h"

Monitoring::Monitoring(): _tcpServer(1), _tcpClient(1), _logger("monitoring.log") {
    _tcpServer.SetOnConnect(std::bind(&Monitoring::_onConnectCallback, this, std::placeholders::_1));
    _tcpServer.SetOnMessage(std::bind(&Monitoring::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpServer.SetOnClose(std::bind(&Monitoring::_onCloseCallback, this, std::placeholders::_1));
    _tcpServer.InitServer(50011, 4096);
    _tcpServer.StartServer();

    _tcpClient.SetOnConnect(std::bind(&Monitoring::_onConnectCallback, this, std::placeholders::_1));
    _tcpClient.SetOnMessage(std::bind(&Monitoring::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpClient.SetOnClose(std::bind(&Monitoring::_onCloseCallback, this, std::placeholders::_1));
    _tcpClient.InitClient(4096);
    _tcpClient.StartClient();
    _fd = _tcpClient.Connect("127.0.0.1", 50001);

    _ipcMsg.resize(TYPE_NUM);
    _uuid.store(0);
    _missWithCenter.store(0);
    _clientPing.store(0);
}

Monitoring::~Monitoring() {
    if(_fd != -1) close(_fd);
    _logger.Log(LOG_INFO, "exit monitoring");
    kill(_rpcPid, SIGKILL);
    waitpid(_rpcPid, nullptr, WNOHANG);
}

void Monitoring::_onConnectCallback(int fd) {

}

void Monitoring::_onCloseCallback(int fd) {
    if(fd == _fd) {
        _tcpClient.Disconnection(fd);
    } else {
        _tcpServer.Disconnection(fd);
    }
}

void Monitoring::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
    HeartBeatProtocol protocol;
    std::vector<char> data(protocol.commHeaderLen, 0);
    if(recvBuf.GetBuffer(protocol.commHeaderLen, data)) {
        protocol.ParseHeader(data);
    } else {
        return;
    }

    data.clear();
    data.resize(protocol.protoMsgLen);
    if(recvBuf.GetBuffer(protocol.protoMsgLen, data)) {
        protocol.ParseBody(data);
    }

    switch(protocol.protoMsgType) {
        case MessageType::PING: // 收到请求
        {
            std::string rsp = HeartBeatProtocol::Build(MessageType::PONG, protocol.protoUUID, _ipcMsg); // TODO：RPC服务进程返回多个score
            _tcpServer.SendMsg(fd, std::vector<char>(rsp.begin(), rsp.end()));
            _clientPing.fetch_add(1);

            _logger.Log(LOG_INFO, "Receive ping from client, uuid: %d", protocol.protoUUID);
            break;
        }
        case MessageType::PONG: // 收到回应
        {
            if(_callbacks.find(protocol.protoUUID) != _callbacks.end()) { // TODO: 线程安全风险
                _callbacks[protocol.protoUUID]();
                _SafeErase(protocol.protoUUID);
            }
            _logger.Log(LOG_INFO, "Receive pong from keeper, uuid: %d", protocol.protoUUID);
            break;
        }
    }
}

void Monitoring::_SafeInsert(uint16_t uuid, std::function<void()> func) {
    std::lock_guard<std::mutex> lock(_callbackMutex);
    _callbacks.emplace(uuid, func);
}

void Monitoring::_SafeErase(uint16_t uuid) {
    std::lock_guard<std::mutex> lock(_callbackMutex);
    _callbacks.erase(uuid);
}

void Monitoring::_RecvMsg() {
    IPCMsg msg;
    int msgNum = 0;
    while(_ipcMsgQueue.RecvMsg(0, msg)) { //  type = 0 读取任意类型
        if(msg.type < TYPE_NUM) {
            _ipcMsg[msg.type] = msg.data;
        }
        msgNum++;
    }
    _logger.Log(LOG_INFO, "Receive status from rpc server by IPC, msgNum: %d", msgNum);
    if(msgNum == 0) { // RPC服务进程没有任何响应
        _KillRpcServer(); // 先杀死，避免是卡住后又恢复
        _StartRpcServer();
    }
}

void Monitoring::_KillRpcServer() {
    _logger.Log(LOG_INFO, "Kill rpc server");
    std::system("pkill rpc_server_main");
    waitpid(0, nullptr, WNOHANG);
}

void Monitoring::_StartRpcServer() {
    _logger.Log(LOG_INFO, "start rpc server");
    pid_t pid = fork();
    if(pid == 0) {
        execl("./rpc_server_main", "rpc_server_main", nullptr);
        perror("execl");
        exit(EXIT_FAILURE);
    } else if(pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else {
        _rpcPid = pid;
    }
}

void Monitoring::_Rescue() {
    _clientPing.store(0);
    if(_missWithCenter.load() < 30) { // 重启tcp连接，10次
        _tcpClient.Disconnection(_fd);
        _fd = _tcpClient.Connect("127.0.0.1", 50001);
    } else if(_missWithCenter.load() < 35) { // 重启网卡，5次
        std::system("ifdown eth0");
        std::system("ifup eth0");
    } else { // 重启机器
        std::system("reboot");
    }

    _logger.Log(LOG_INFO, "loss connection with keeper, rescue");
}

void Monitoring::_SendHeartBeat() {
    uint16_t uuid = _uuid.fetch_add(1);
    TimerPara timerPara = _timer.AddTimer(3333, [&](){
        std::cout << __func__ << "<<< lose heart beat" << std::endl;
        if(_missWithCenter.fetch_add(1) >= 20) {
            _Rescue();
        }
    }, false);

    _SafeInsert(uuid, [timerPara,this]() {
        _timer.DeleteTimer(timerPara);
        _missWithCenter.store(0);
    });

    std::string req = HeartBeatProtocol::Build(MessageType::PING, uuid);
    _tcpClient.SendMsg(_fd, std::vector<char>(req.begin(), req.end()));
    _logger.Log(LOG_INFO, "send heart beat to keeper, uuid: %d", uuid);
}

void Monitoring::StartMonitoring() {
    _missWithCenter.store(1); // 用于表示未连接

    _timer.AddTimer(5678, std::bind(&Monitoring::_SendHeartBeat, this), true);

    std::thread th([&](){
        while(_missWithCenter.load() != 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        signal(SIGCHLD, SIG_IGN); // 避免僵尸进程
        _StartRpcServer();
        _timer.AddTimer(12345, std::bind(&Monitoring::_RecvMsg, this), true);
    });

    _timer.RunTimer();
}