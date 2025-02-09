#include "log.h"
#include <iostream>
#include <utility>

namespace sylar {

// TODO: this function can be optimized by using emplace
void Logger::add_appender(LogAppender::ptr appender) { _appenders.push_back(appender); }
void Logger::del_appender(LogAppender::ptr appender)
{
#if 0
    for (auto iter = _appenders.begin(); iter != _appenders.end(); iter++) {
        if (*iter != appender) {
            _appenders.erase(iter);
            break;
        }
    }
#endif // 0
    _appenders.remove_if([&](auto iter) { return iter != appender; });
}

Logger::Logger(std::string name)
    : _name { std::move(name) }
{
}
void Logger::log(LogLevel level, LogEvent::ptr event)
{
    // only outputs higher logs of higher level
    if (level >= _level) {
        for (auto& iter : _appenders) {
            iter->log(level, event);
        }
    }
}

// TODO: diff
void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }
void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }
void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }
void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }
void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

bool FileLogAppender::reopen()
{
    if (_filestream) {
        _filestream.close();
    }
    _filestream.open(_filename);

    // 学
    return !!_filestream;
}

FileLogAppender::FileLogAppender(const std::string& filename)
    : _filename { filename }
{
}

void FileLogAppender::log(LogLevel level, LogEvent::ptr event)
{
    if (level >= _level) {
        _filestream << _formatter->format(event);
    }
}

void StdoutAppender::log(LogLevel level, LogEvent::ptr event)
{
    if (level >= _level) {
        std::cout << _formatter->format(event);
    }
}

}
