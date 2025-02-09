#include "log.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace sylar {

const char* LogLevel::to_string(LogLevel::Level level)
{
#if 0
    
    switch (level) {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;

        XX(UNKNOWN);
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);

#undef XX

    default:
        return "UNKNOW";
    }
    return "UNKNOW";
#endif // 0
    static constexpr std::array<const char*, 6> level_strings = {
        "UNKNOWN", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };

    // 将枚举转换为下标
    auto index = static_cast<std::size_t>(level);
    if (index < level_strings.size()) {
        return level_strings.at(index);
    }
    return "UNKNOWN";
}

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
void Logger::log(LogLevel::Level level, LogEvent::ptr event)
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

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= _level) {
        _filestream << _formatter->format(event);
    }
}

void StdoutAppender::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= _level) {
        std::cout << _formatter->format(event);
    }
}

LogFormatter::LogFormatter(const std::string& pattern)
    : _pattern { pattern }
{
}

void LogFormatter::init()
{
    // str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < _pattern.size(); ++i) {
        if (_pattern[i] != '%') {
            nstr.append(1, _pattern[i]);
            continue;
        }

        if ((i + 1) < _pattern.size()) {
            if (_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while (n < _pattern.size()) {
            if (!fmt_status && (!isalpha(_pattern[n]) && _pattern[n] != '{' && _pattern[n] != '}')) {
                str = _pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (_pattern[n] == '{') {
                    str = _pattern.substr(i + 1, n - i - 1);
                    // std::cout << "*" << str << std::endl;
                    fmt_status = 1; // 解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1) {
                if (_pattern[n] == '}') {
                    fmt = _pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    // std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == _pattern.size()) {
                if (str.empty()) {
                    str = _pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << _pattern << " - " << _pattern.substr(i) << std::endl;
            _error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    // std::cout << m_items.size() << std::endl;
}

std::string LogFormatter::format(LogLevel::Level level, LogEvent::ptr event)
{
    std::stringstream ss;
    for (auto& iter : _items) {
        iter->format(ss, level, event);
    }
    return ss.str();
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_content();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << LogLevel::to_string(level);
    }
};

}
