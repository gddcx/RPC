#ifndef _RPC_PROCESSOR_H_
#define _RPC_PROCESSOR_H_

#include <vector>
#include <unordered_map>

#include "threadpool.h"
#include "rpc_acceptor.h"

namespace crpc {

class RpcProcessor {
private:
    ThreadPool _threadPool;
    RpcAcceptor* _rpcAcceptor;
    std::unordered_map<uint16_t, std::function<std::string(std::string)>> _serviceTable;
    std::vector<std::function<void()>> _futureVec;
public:
    RpcProcessor(int threadNum);
    void SetAcceptor(RpcAcceptor* rpcAcceptor);
    void Process(int fd, int requestId, uint16_t serviceIndex, std::string& data);
    void MessageRegister(uint16_t serviceId, std::function<std::string(std::string)> handler);
};

}

#endif