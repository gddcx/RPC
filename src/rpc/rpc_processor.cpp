#include <chrono>
#include <functional>
#include <string>
#include "rpc_protocol.h"
#include "rpc_processor.h"

namespace crpc {

RpcProcessor::RpcProcessor(int threadNum): _threadPool(threadNum) {
    _threadPool.Init();
}

void RpcProcessor::SetAcceptor(RpcAcceptor* rpcAcceptor) {
    _rpcAcceptor = rpcAcceptor;
    _rpcAcceptor->Process = std::bind(&RpcProcessor::Process, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

void RpcProcessor::Process(int fd, int requestId, uint8_t msgType, std::string& data) {
    if(_serviceTable.find((msgType)) != _serviceTable.end()) {
        auto future = _threadPool.Submit(
            [&](std::string& para){
                std::string res = _serviceTable[msgType](para);
                RpcProtocol::Build(requestId, msgType, res);
                this->_rpcAcceptor->SendMsg(fd, res);
            },
            data
        );
    } else {
        std::cout << __func__ << ">>> msgType:" << (int)msgType << " has no corresponsding service function" << std::endl;
    }
}

void RpcProcessor::MessageRegister(uint8_t serviceId, std::function<std::string(std::string)> handler) {
    _serviceTable.insert(std::make_pair(serviceId, handler));
}

}