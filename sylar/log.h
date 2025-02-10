
#pragma once

#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace sylar {

class Logger;

class LogLevel {
public:
    enum Level : uint8_t {
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5,
    };

    static const char* to_string(LogLevel::Level level);
};

// log event
class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent();
    /**
     * @brief 返回文件名
     */
    [[nodiscard]] const char* get_file() const { return _file; }

    /**
     * @brief 返回行号
     */
    [[nodiscard]] int32_t get_line() const { return _line; }

    /**
     * @brief 返回耗时
     */
    [[nodiscard]] uint32_t get_elapse() const { return _elapse; }

    /**
     * @brief 返回线程ID
     */
    [[nodiscard]] uint32_t get_thread_id() const { return _thread_id; }

    /**
     * @brief 返回协程ID
     */
    [[nodiscard]] uint32_t get_fiberId() const { return _fiber_id; }

    /**
     * @brief 返回时间
     */
    [[nodiscard]] uint64_t get_time() const { return _time; }

    [[nodiscard]] std::string get_content() const { return _content; }

private:
    const char* _file = nullptr; // file name
    int32_t _line = 0; // the number of line
    uint32_t _thread_id = 0; // thread id
    uint32_t _fiber_id = 0; // fiber id
    int64_t _time = 0; // time stamp
    uint32_t _elapse = 0; // millseconds since the program started
    std::string _content;
};

class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;

    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    LogFormatter(const std::string& pattern);

    class FormatItem {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        // explicit FormatItem(const std::string& fmt = "") {};
        virtual ~FormatItem() = default;
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

private:
    void init();
    std::string _pattern;
    std::vector<FormatItem::ptr> _items;
    bool _error;
};

// log output location
class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() = default;

    virtual void log(std::shared_ptr<Logger>, LogLevel::Level level, LogEvent::ptr event) = 0;
    void set_formatter(LogFormatter::ptr val) { _formatter = val; }
    [[nodiscard]] LogFormatter::ptr get_fotmatter() const { return _formatter; }

protected:
    LogLevel::Level _level;
    LogFormatter::ptr _formatter;
};

// log output device
class Logger : public std::enable_shared_from_this<Logger> {
public:
    using ptr = std::shared_ptr<Logger>;

    explicit Logger(std::string name = "root");

    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void add_appender(LogAppender::ptr appender);
    void del_appender(LogAppender::ptr appender);
    [[nodiscard]] LogLevel::Level get_level() const { return _level; }
    [[nodiscard]] const std::string& get_name() const { return _name; }
    void set_level(LogLevel::Level level) { _level = level; }

private:
    std::string _name; // name of log
    LogLevel::Level _level; // log if level fit the _level here
    std::list<LogAppender::ptr> _appenders; // the lists of appenders
    LogFormatter::ptr _formatter;
};

// output to command line
class StdoutAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutAppender>;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

private:
};

// output to file
class FileLogAppender : public LogAppender {

public:
    using ptr = std::shared_ptr<FileLogAppender>;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    explicit FileLogAppender(const std::string& filename);

    // if open successfully, return true
    bool reopen();

private:
    std::string _filename;
    std::ofstream _filestream;
};

}
