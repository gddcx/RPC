#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <sstream>
#include "logging.h"

Logger::Logger(const std::string& fileName) {
    _logFile.open(fileName, std::ios::app);
}

Logger::~Logger() {
    if(_logFile.is_open()) {
        _logFile.close();
    }
}

std::string Logger::CurrentTime() {
    time_t now = time(0);
    tm* tm_t = localtime(&now);
    std::stringstream ss;
    ss << tm_t->tm_year + 1900 << "-" << tm_t->tm_mon + 1 << "-" << tm_t->tm_mday << " " << tm_t->tm_hour << ":" << tm_t->tm_min << ":" << tm_t->tm_sec;
    return ss.str();
}

void Logger::Log(LoggerLevel level, const char* format, ...) {
    char buffer[128] = {0};
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::lock_guard<std::mutex> lock(_mutex);
    _logFile << "[ " << CurrentTime() << " ] " << buffer << std::endl;
}