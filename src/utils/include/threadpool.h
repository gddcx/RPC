#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <condition_variable>
#include <functional>
#include <thread>
#include <future>
#include <iostream>
#include <string>
#include "safe_queue.h"
class ThreadPool {
private:
    class ThreadWorker {
    public:
        ThreadWorker(int id, ThreadPool* pool): _threadId(id), _pool(pool) {}
        void operator()() {
            std::function<void()> func;
            bool hasTask;
            while(true) {
                {
                    std::unique_lock<std::mutex> lock(_pool->_threadMutex);
                    _pool->_threadCv.wait(lock, [&](){return !_pool->_taskQue.empty();});
                    hasTask = _pool->_taskQue.fetch(func);
                }
                if(hasTask) {
                    func();
                }
            }
        }
    private:
        int _threadId;
        ThreadPool* _pool;
    };
public:
    ThreadPool(int threadNum): _threads(threadNum) {}
    void Init() {
        for(size_t tid = 0; tid < _threads.size(); tid++) {
            _threads[tid] = std::thread(ThreadWorker(tid, this));
        }
    }

    void Close() {
        _threadCv.notify_all();
        for(size_t tid = 0; tid < _threads.size(); tid++) {
            if(_threads[tid].joinable()) {
                _threads[tid].join();
            }
        }
    }

    template<typename F, typename ...Args>
    auto Submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>{
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...); // 完美转发
        /* 避免packaged_task生成的对象因为submit退出而销毁 */
        auto taskPtr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void()> warpperFunc = [taskPtr]() {
            (*taskPtr)();
        };

        _taskQue.append(warpperFunc);
        _threadCv.notify_one();

        return taskPtr->get_future();
    }

private:
    SafeQueue<std::function<void()>> _taskQue;
    std::condition_variable _threadCv;
    std::mutex _threadMutex; // 没有实际作用
    std::vector<std::thread> _threads;
};

#endif