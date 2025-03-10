#ifndef _RPC_BALANCE_H_
#define _RPC_BALANCE_H_

#include <unordered_map>
#include <utility>
#include <mutex>
#include <array>
#include "keeper_client.h"
#include "tcp_client.h"
#include "timer.h"
#include "random_umap.h"
#include "service_discovery.h"

/*
    1. 向注册中心要提供服务的[服务节点]列表 done
    2. 起定时器，定时向[服务节点]发送心跳
    3. 配置各个机器的权重 
    4. 为rpc client的请求选择[服务节点] done
*/

class RpcBalancer {
private:
    void _onMessageCallback(int fd, RecvBuffer& recvBuf);
    void _onCloseCallback(int fd);
    void _SafeInsertCache(uint16_t serviceIndex, const std::pair<uint32_t, uint16_t>& ipAndPort);
    void _SafeInsertCallback(uint32_t uuid, std::function<void(uint8_t)> func);
    void _SafeEraseCallback(uint32_t uuid);
    std::pair<uint32_t, uint16_t> _NodeSeletion(uint16_t serviceIndex);
    void _SendHeartBeat();
public:
    RpcBalancer();
    ~RpcBalancer();
    void SetKeeper(const std::string& ip, uint16_t port);
    std::pair<uint32_t, uint16_t> FetchServiceNode(uint16_t serviceIndex);
private:
    std::atomic<uint32_t> _uuid;
    std::mutex _cacheMutex;
    std::mutex _callbacksMutex;
    KeeperClient* _keeperClient = nullptr;  // 连接到注册中心
    TcpClient _tcpClient;   // 连接到RPC服务端
    Timer _timer;
    std::thread _timerThread;
    /* ==============和rpc server通信相关===================*/
    std::unordered_map<uint16_t, std::unordered_set<std::pair<uint32_t, uint16_t>, SetCmp>> _serviceCache; // serviceID: {{IP,Port}}
    std::unordered_map<uint32_t, std::function<void(uint8_t)>> _callbacks; // uuid: callback
    enum AbilityType {SCORE, RSP_TIME, ABILITY_NUM};
    RandomUMap<uint32_t, std::vector<int8_t>> _nodeAbility; // IP: [score, RSP_TIME]
    std::unordered_map<uint32_t, uint16_t> _nodeConnCnt;
    /* ==============和monitoring通信相关===================*/
    std::unordered_map<uint32_t, int> _connCacheIP2Fd; // IP: fd
    std::unordered_map<int, uint32_t> _connCacheFd2IP; // fd: IP
};

#endif