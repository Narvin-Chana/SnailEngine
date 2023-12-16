#include "stdafx.h"
#include "Logger.h"

#include <chrono>

namespace Snail
{
std::ostream& timestamp(std::ostream& os)
{
    os << std::chrono::system_clock::now();
    return os;
}

LogException::LogException(const std::string& errorLog)
    : errorLog{errorLog}
{
}

const char* LogException::what() const noexcept
{
    return errorLog.c_str();
}

void Logger::SetLogFile(const std::string& filename)
{
    out = std::ofstream{filename};
}

std::ostream& operator<<(std::ostream& os, Logger::LogLevel lvl)
{
    static const char* names[] = { "INFO", "ERROR", "WARN" };
    os << names[static_cast<std::underlying_type_t<Logger::LogLevel>>(lvl)];
    return os;
}
}
