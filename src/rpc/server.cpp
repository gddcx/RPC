#include <chrono>
#include "server.h"
#include "rpc_invoker.h"
// 1. 网络库接收数据
// 2. 反序列化，按协议解析
// 3. 解析后数据放入到任务队列，唤醒工作线程t1
// 4. 工作线程t1找到对应服务接口，执行
// 5. 接口执行结果序列化
// 6. 调用网络库发送

namespace crpc {

RpcServer::RpcServer(int netThread, int serviceThread): _rpcProcessor(serviceThread), _rpcAcceptor(netThread), _keeperClient(1) {
    _rpcProcessor.SetAcceptor(&_rpcAcceptor);
}

int RpcServer::Main() {
    _keeperClient.RegisterService("TestFunc", "127.0.0.1", "50010");
    _rpcProcessor.MessageRegister(1, TestFunc);
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}

}