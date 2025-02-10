#include "sylar/log.h"
#include <iostream>

using namespace sylar;

int main(int argc, char* argv[])
{
    Logger::ptr logger { new Logger };
    logger->add_appender(LogAppender::ptr(new sylar::StdoutAppender));

    LogEvent::ptr event(new LogEvent(__FILE__, __LINE__, 1, 2, 3, time(nullptr)));

    logger->log(sylar::LogLevel::DEBUG, event);

    std::cout << "\n--------------\n";

    event->get_ss() << "*******";
    std::cout << event->get_content();

    return 0;
}
