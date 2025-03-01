#include <iostream>
#include <thread>
#include "timer.h"

Timer::Timer() {
    _timerID.store(0);
    _stop = false;
    // 设一个空任务，有两个作用：1. 无实际任务时用于线程sleep，释放CPU；2. 避免第一个任务为长周期任务，导致后面加入的任务要等太久
    AddTimer(3000, [](){}, true);
}

time_t Timer::_GetTick() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

const TimerPara Timer::AddTimer(uint32_t periodMs, TimerTask callback, bool keep) {
    uint32_t expired = periodMs + _GetTick();
    uint32_t timerID = _timerID.fetch_add(1);
    TimerPara timerPara(timerID, expired, periodMs, callback, keep);
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _timerSet.insert(timerPara);
    }
    return timerPara;
}

void Timer::DeleteTimer(const TimerPara& timerPara) {
    std::lock_guard<std::mutex> lock(_mutex);
    if(_timerSet.find(timerPara) != _timerSet.end()) {
        _timerSet.erase(timerPara);
    }
}

TimerPara Timer::GetTimer() {
    std::lock_guard<std::mutex> lock(_mutex);
    return *_timerSet.begin();
}

void Timer::_WaitTimer() {
    time_t tick = _GetTick();
    uint32_t timeDiff = _timerSet.begin()->_expired - tick;
    if(timeDiff > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeDiff));
    }
}

void Timer::_ExecuteTask() {
    time_t tick = _GetTick();
    TimerPara para = GetTimer();
    DeleteTimer(para);
    if((para._expired - tick) <= 0) {
        para.task();
        if(para._keep) AddTimer(para._period, para.task, para._keep);
    }
}

void Timer::RunTimer() {
    while(true) {
        _WaitTimer();
        _ExecuteTask();
        if(_stop) break;
    }
}

void Timer::StopTimer() {
    _stop = true;
}