#ifndef _TIMER_H_
#define _TIMER_H_

#include <unordered_map>
#include <set>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>

using TimerTask = std::function<void()>;

struct TimerPara {
    uint32_t _timerID;
    uint32_t _expired;
    uint32_t _period;
    TimerTask task;
    bool _keep;
    bool operator<(const TimerPara& para) const {
        if(_expired < para._expired) {
            return true;
        } else if (_expired > para._expired) {
            return false;
        } else {
            return _timerID > para._timerID;
        }
    }

    TimerPara() = default;
    TimerPara(uint32_t timerID, uint32_t expired, uint32_t period, TimerTask tsk, bool keep): 
        _timerID(timerID), _expired(expired), _period(period), task(tsk), _keep(keep) {}
};

class Timer {
public:
    Timer();
    const TimerPara AddTimer(uint32_t period, TimerTask timerTask, bool keep);
    void DeleteTimer(const TimerPara& timerPara);
    void RunTimer();
    void StopTimer();
private:
    time_t _GetTick();
    void _WaitTimer();
    void _ExecuteTask();
    TimerPara GetTimer();
private:
    bool _stop;
    std::mutex _mutex;
    std::atomic<uint32_t> _timerID;
    std::set<TimerPara> _timerSet;
};

#endif