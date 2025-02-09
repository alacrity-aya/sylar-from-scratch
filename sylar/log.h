
#pragma once

#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <string>
namespace sylar {

enum class LogLevel : uint8_t {
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
};

// log event
class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent();

private:
    const char* _file = nullptr; // file name
    int32_t _line = 0; // the number of line
    uint32_t thread_id = 0; // thread id
    uint32_t fiber_id = 0; // fiber id
    int64_t _time = 0; // time stamp
    uint32_t _elapse = 0; // millseconds since the program started
    std::string _content;
};

class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;

    std::string format(LogEvent::ptr event);

private:
};

// log output location
class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() = default;

    virtual void log(LogLevel level, LogEvent::ptr event) = 0;
    void set_formatter(LogFormatter::ptr val) { _formatter = val; }
    [[nodiscard]] LogFormatter::ptr get_fotmatter() const { return _formatter; }

protected:
    LogLevel _level;
    LogFormatter::ptr _formatter;
};

// log output device
class Logger {
public:
    using ptr = std::shared_ptr<Logger>;

    explicit Logger(std::string name = "root");

    void log(LogLevel level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void add_appender(LogAppender::ptr appender);
    void del_appender(LogAppender::ptr appender);
    [[nodiscard]] LogLevel get_level() const { return _level; }
    void set_level(LogLevel level) { _level = level; }

private:
    std::string _name; // name of log
    LogLevel _level; // log if level fit the _level here
    std::list<LogAppender::ptr> _appenders; // the lists of appenders
};

// output to command line
class StdoutAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutAppender>;
    void log(LogLevel level, LogEvent::ptr event) override;

private:
};

// output to file
class FileLogAppender : public LogAppender {

public:
    using ptr = std::shared_ptr<FileLogAppender>;
    void log(LogLevel level, LogEvent::ptr event) override;
    explicit FileLogAppender(const std::string& filename);

    // if open successfully, return true
    bool reopen();

private:
    std::string _filename;
    std::ofstream _filestream;
};

}
