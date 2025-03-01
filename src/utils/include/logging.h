#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <fstream>
#include <mutex>
#include <string>

enum LoggerLevel {
    LOG_DEBUF,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

class Logger {
private:
    std::string CurrentTime();
public:
    Logger(const std::string& fileName);
    ~Logger();
    void Log(LoggerLevel level, const char* format, ...);
private:
    std::ofstream _logFile;
    std::mutex _mutex;
};
#endif