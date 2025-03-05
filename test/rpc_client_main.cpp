#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>
#include "client.h"
#include "keeper_client.h"
#include "rpc_balancer.h"

using namespace std;
using namespace chrono;
using namespace crpc;


atomic<int> success_count(0); // 记录成功请求数
atomic<int> failed_count(0);  // 记录失败请求数
mutex cout_mutex;

// 发送 RPC 请求的函数
void send_rpc_request(int thread_id, int request_count) {
    RpcBalancer rpcBalancer;
    rpcBalancer.SetKeeper("127.0.0.1", 50001);
    std::pair<uint32_t, uint16_t> ipPort = rpcBalancer.FetchServiceNode(0);
    if(ipPort.first == 0) {
        std::cout << "fetch service node err" << std::endl;
        return;
    }

    RpcClient rpcClient(1);

    RPCMsg::Request req;

    req.set_userid(55);
    req.set_hostname("cx");

    for (int i = 0; i < request_count; i++) {
        RPCMsg::Response rsp;
        // auto start = high_resolution_clock::now();
        
        try{
            rpcClient.RpcFunc1(req, rsp, ipPort);
        } catch (const std::runtime_error& e) {
            std::cout << e.what() << std::endl;
        }

        // auto end = high_resolution_clock::now();
        // auto duration = duration_cast<milliseconds>(end - start).count();

        if (rsp.stat() == 33) {
            success_count++;
        } else {
            failed_count++;
        }
        
        // if((i % 100) == 0)
        // {
        //     lock_guard<mutex> lock(cout_mutex);
        //     cout << "Thread " << thread_id << " - Request " << i << " - Time: " << duration << "ms" << endl;
        // }
    }
}

int main(int argc, char *args[]) {
    const int thread_count = atoi(args[1]); // 线程数（并发数）
    const int requests_per_thread = atoi(args[2]); // 每个线程的请求数

    vector<thread> threads;
    auto start_time = high_resolution_clock::now();

    // 创建多个线程发送请求
    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(send_rpc_request, i, requests_per_thread);
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    auto end_time = high_resolution_clock::now();
    std::cout << "get end time" << std::endl;
    auto total_duration = duration_cast<milliseconds>(end_time - start_time).count();
    std::cout << "total_duration" << std::endl;
    // 输出测试结果
    cout << "Total Requests: " << (success_count + failed_count) << endl;
    cout << "Successful Requests: " << success_count.load() << endl;
    cout << "Failed Requests: " << failed_count.load() << endl;
    cout << "Total Time: " << total_duration << " ms" << endl;
    cout << "QPS: " << (success_count.load() * 1000 / total_duration) << " requests/sec" << endl;

    std::cout.flush();
    return 0;
}
