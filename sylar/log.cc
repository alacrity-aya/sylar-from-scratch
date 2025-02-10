#include "log.h"
#include <functional>
#include <iostream>
#include <map>
#include <memory>
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
void Logger::add_appender(LogAppender::ptr appender)
{
    if (!appender->get_fotmatter()) {
        appender->set_formatter(_formatter);
    }
    _appenders.push_back(appender);
}
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
    , _formatter { std::make_shared<LogFormatter>("%d  [%p] %f %l %m %n") }

{
}
void Logger::log(LogLevel::Level level, LogEvent::ptr event)
{
    // only outputs higher logs of higher level
    if (level >= _level) {
        // TODO: learn shared_from_this
        auto self = shared_from_this();
        for (auto& iter : _appenders) {
            iter->log(self, level, event);
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

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= _level) {
        _filestream << _formatter->format(logger, level, event);
    }
}

void StdoutAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= _level) {
        std::cout << _formatter->format(logger, level, event);
    }
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& fmt = "") { }

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_content();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << LogLevel::to_string(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_elapse();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_thread_id();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_fiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    // optimized by using std::move
    explicit DateTimeFormatItem(std::string format = "%Y:%m:%d %H:%M:%S")
        : _format { std::move(format) }
    {
    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_time();
    }

private:
    std::string _format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_file();
    }
};
class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->get_line();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        // os << "\n";
        os << std::endl;
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << '\t';
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& fmt = "") { }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << logger->get_name();
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    explicit StringFormatItem(std::string string)
        : _string(std::move(string))
    {
    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << _string;
    }

private:
    std::string _string;
};

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
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items
    {

#define XX(str, C) \
    { #str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); } }

        XX(m, MessageFormatItem), // m:消息
            XX(p, LevelFormatItem), // p:日志级别
            XX(r, ElapseFormatItem), // r:累计毫秒数
            XX(c, NameFormatItem), // c:日志名称
            XX(t, ThreadIdFormatItem), // t:线程id
            XX(n, NewLineFormatItem), // n:换行
            XX(d, DateTimeFormatItem), // d:时间
            XX(f, FilenameFormatItem), // f:文件名
            XX(l, LineFormatItem), // l:行号
            XX(T, TabFormatItem), // T:Tab
            XX(F, FiberIdFormatItem), // F:协程id
        // XX(N, ThreadNameFormatItem), // N:线程名称
#undef XX
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            _items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                _items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                // m_error = true;
            } else {
                _items.push_back(it->second(std::get<1>(i)));
            }
        }

        // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    std::stringstream ss;
    for (auto& iter : _items) {
        iter->format(ss, logger, level, event);
    }
    return ss.str();
}

}

#if 0
int main(int argc, char* argv[])
{
    std::cout << "hello";
    return 0;
}
#endif
