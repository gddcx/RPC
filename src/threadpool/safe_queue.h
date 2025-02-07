#ifndef _SAVE_QUEUE_H_
#define _SAVE_QUEUE_H_

#include <queue>
#include <mutex>

template<typename T>
class SafeQueue {
public:
    void append(T& data) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(data);
    }

    bool fetch(T& data) {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_queue.empty()) {
            return false;
        }
        data = std::move(_queue.front());
        _queue.pop();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    int size() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }
private:
    std::queue<T> _queue;
    std::mutex _mutex;
};

#endif