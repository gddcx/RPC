#include <float.h>
#include <iostream>
#include <functional>
#include <arpa/inet.h>
#include <set>
#include <algorithm>
#include "rpc_balancer.h"
#include "heart_beat_protocol.h"

RpcBalancer::RpcBalancer(): _tcpClient(1) {
    _uuid.store(0);

    _timerThread = std::thread(&Timer::RunTimer, &_timer);

    _tcpClient.SetOnMessage(std::bind(&RpcBalancer::_onMessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    _tcpClient.SetOnClose(std::bind(&RpcBalancer::_onCloseCallback, this, std::placeholders::_1));
    _tcpClient.InitClient(4096);
    _tcpClient.StartClient();

    _timer.AddTimer(5000, [this](){
        _SendHeartBeat();
    }, true);
}

RpcBalancer::~RpcBalancer() {
    _timer.StopTimer();
    if(_timerThread.joinable()) {
        _timerThread.join();
    }
    if(_keeperClient != nullptr) delete _keeperClient;
}

void RpcBalancer::SetKeeper(const std::string& ip, uint16_t port) {
    _keeperClient = new KeeperClient(ip, port, 1);
}

void RpcBalancer::_onCloseCallback(int fd) {
    uint32_t ip = _connCacheFd2IP[fd];
    _connCacheFd2IP.erase(fd);
    _connCacheIP2Fd.erase(ip);
    _tcpClient.Disconnection(fd);
}

void RpcBalancer::_onMessageCallback(int fd, RecvBuffer& recvBuf) {
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
        case MessageType::PING:
        {
            break;
        }
        case MessageType::PONG: // 收到回应
        {
            if(_callbacks.find(protocol.protoUUID) != _callbacks.end()) {
                _callbacks[protocol.protoUUID](protocol.payLoad[HeartBeatProtocol::SCORE1]); // TODO:和超时定时器的删除操作要互斥
                _SafeEraseCallback(protocol.protoUUID);
            }
            break;
        }
    }
}

void RpcBalancer::_SendHeartBeat() {
    uint32_t uuid = _uuid.fetch_add(1);
    uint32_t ip = 0;
    std::set<uint32_t> hasSent;
    /* 最多随机抽取5个IP发心跳 */
    int fd = -1;
    int ipNum = std::min((uint32_t)5, _nodeAbility.size());
    for(int n = 0; n < ipNum; n++) {
        ip = _nodeAbility.RandomGetKey();
        while(hasSent.find(ip) != hasSent.end()) {
            ip = _nodeAbility.RandomGetKey();
        }

        if(_connCacheIP2Fd.find(ip) != _connCacheIP2Fd.end()) { // TODO: 线程安全
            fd = _connCacheIP2Fd[ip];
        } else {
            sockaddr_in addr;
            addr.sin_addr.s_addr = ip;
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ipStr, INET_ADDRSTRLEN);
            fd = _tcpClient.Connect(ipStr, 50011);
            if(fd < 0) {
                continue;
            }
            _connCacheIP2Fd[ip] = fd;
            _connCacheFd2IP[fd] = ip;
        }

        TimerPara timerPara = _timer.AddTimer(3000, [this,ip,uuid](){ /* TODO:改造返回值，不要用Timepara，用shared_ptr */
            _nodeAbility[ip] /= 2; // TODO：心跳丢失，节点分数变更策略
            _SafeEraseCallback(uuid);
        }, false);

        _SafeInsertCallback(uuid, [timerPara,ip,this](uint8_t score) {
            _timer.DeleteTimer(timerPara);
            _nodeAbility[ip] = score;
        });

        std::string req = HeartBeatProtocol::Build(MessageType::PING, uuid);
        _tcpClient.SendMsg(fd, std::vector<char>(req.begin(), req.end()));

        hasSent.insert(ip);
    }
}

void RpcBalancer::_SafeEraseCallback(uint32_t uuid) {
    std::lock_guard<std::mutex> lock(_callbacksMutex);
    _callbacks.erase(uuid);
}

void RpcBalancer::_SafeInsertCallback(uint32_t uuid, std::function<void(uint8_t)> func) {
    std::lock_guard<std::mutex> lock(_callbacksMutex);
    _callbacks[uuid] = func;
}

std::pair<uint32_t, uint16_t> RpcBalancer::FetchServiceNode(uint16_t serviceIndex) {
    // TODO：要避免并发时重复发送请求
    if(_serviceCache.find(serviceIndex) == _serviceCache.end()) {
        auto future = _keeperClient->FetchService(serviceIndex);
        auto IpPorts = future.get();
        _serviceCache[serviceIndex].insert(IpPorts.begin(), IpPorts.end());
        for(auto& machine: IpPorts) {
            if(_nodeAbility.find(machine.first) == _nodeAbility.end()) {
                _nodeAbility[machine.first] = 10;
            }
        }
    }

    /* TODO: 缓存服务节点少于n时需要重新向注册中心请求节点 */
    if(_serviceCache[serviceIndex].size() == 0) {
        return std::pair<uint32_t, uint16_t>(0, 0);
    }
    return _NodeSeletion(serviceIndex);
}

void RpcBalancer::_SafeInsertCache(uint16_t serviceIndex, const std::pair<uint32_t, uint16_t>& ipAndPort) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    _serviceCache[serviceIndex].insert(ipAndPort);
}

std::pair<uint32_t, uint16_t> RpcBalancer::_NodeSeletion(uint16_t serviceIndex) {
    uint32_t ip = 0;
    float tmpAbility = 0.0;
    float ability = FLT_MIN;

    const std::pair<uint32_t, uint16_t>* sel;
    const std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>& nodeList = _serviceCache[serviceIndex];
    for(const auto& p: nodeList) {
        ip = p.first;
        // 要避免羊群效应(惊群问题)：因为节点状态不是实时更新，如果判断某个节点状态好就一下子把所有请求到给到这个节点，反而会导致这个节点负载过高。
        tmpAbility = (float)_nodeAbility[ip] / (_nodeConnCnt[ip] == 0 ? 1 : _nodeConnCnt[ip]);
        if(tmpAbility >= ability) {
            ability = tmpAbility;
            sel = &p;
        }
    }

    return *sel;
}