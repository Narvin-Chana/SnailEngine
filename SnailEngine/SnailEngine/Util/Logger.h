#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <format>

#include "SnailException.h"
#include "Util/DebugUtil.h"

// Macro declared in windgi.h which is unused here
#undef ERROR

namespace Snail
{

class LogException : public SnailException
{
    std::string errorLog;
public:
    LogException(const std::string& errorLog);
    const char* what() const noexcept override;
};

class Logger
{
    std::mutex logMutex;
    std::ofstream out;
public:
    enum LogLevel{ INFO, ERROR, WARN, FATAL };

    Logger() = default;
    void SetLogFile(const std::string& filename);

    template<class T>
    friend Logger& operator<<(Logger& logger, T&& val);

    template<class ...T>
    void Log(T&& ... args);

    template<class ...T>
    void Log(LogLevel level, T&& ... args);

    // uses the std::format API
    // https://en.cppreference.com/w/cpp/utility/format/format
    template<class ...T>
    void Logf(std::format_string<T...> format, T&& ... args);

    // uses the std::format API
    // https://en.cppreference.com/w/cpp/utility/format/format
    template<class ...T>
    void Logf(LogLevel level, std::format_string<T...> format, T&& ... args);

    friend std::ostream& operator<<(std::ostream& os, LogLevel lvl);
};

std::ostream& timestamp(std::ostream& os);

template <class ... T>
void Logger::Log(T&&... args)
{
    Log(INFO, std::forward<T>(args)...);
}

template <class ... T>
void Logger::Log(LogLevel level, T&&... args)
{
    std::lock_guard lock{ logMutex };

    std::stringstream output;
    output << '[' << timestamp << "][" << level << "]: ";
    ((output << args), ...);
    output << '\n';
    std::string errMsg = output.str();
#ifdef _DEBUG
    DebugPrint(errMsg);
#endif
    out << errMsg;

    if (level == FATAL)
        throw LogException(errMsg);
}

template <class ... T>
void Logger::Logf(LogLevel level, std::format_string<T...> format, T&&... args)
{
    std::lock_guard lock{ logMutex };
    std::string logMsg = std::format(format, std::forward<T>(args)...);
#ifdef _DEBUG
    DebugPrint('[', timestamp, "][", level, "]: ", logMsg, '\n');
#endif
    out << '[' << timestamp << "][" << level << "]: " << logMsg << std::endl;

    if (level == FATAL)
        throw LogException(logMsg);
}

template <class ... T>
void Logger::Logf(std::format_string<T...> format, T&&... args)
{
    Logf(
        INFO,
        std::forward<std::format_string<T...>>(format),
        std::forward<T>(args)...
    );
}

inline Logger logger{};

// uses the std::format API
// https://en.cppreference.com/w/cpp/utility/format/format
template <class ... T>
void LOGF(std::format_string<T...> format, T&&... args)
{
    logger.Logf(
        std::forward<std::format_string<T...>>(format),
        std::forward<T>(args)...
    );
}

// uses the std::format API
// https://en.cppreference.com/w/cpp/utility/format/format
template <class ... T>
void LOGF(Logger::LogLevel level, std::format_string<T...> format, T&&... args)
{
    logger.Logf(
        level,
        std::forward<std::format_string<T...>>(format),
        std::forward<T>(args)...
    );
}

template <class ... T>
void LOG(T&&... args)
{
    logger.Log(std::forward<T>(args)...);
}

template <class ... T>
void LOG(Logger::LogLevel level, T&&... args)
{
    logger.Log(level, std::forward<T>(args)...);
}
}
