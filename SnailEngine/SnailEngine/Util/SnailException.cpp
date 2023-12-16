#include "stdafx.h"
#include "Util/SnailException.h"

#include <sstream>

namespace Snail
{
SnailException::SnailException(const unsigned line, const char* file) noexcept :
    line{ line }
    , file{ file }
{}

SnailException::SnailException(const char* errorMessage) noexcept : whatBuffer(errorMessage)
{}

const char* SnailException::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* SnailException::GetType() const noexcept
{
    return "Snail Exception";
}

unsigned int SnailException::GetLine() const noexcept
{
    return line;
}

const std::string& SnailException::GetFile() const noexcept
{
    return file;
}

std::string SnailException::GetOriginString() const noexcept
{
    std::ostringstream oss;
    oss << "[File] " << file << std::endl
        << "[Line] " << line;
    return oss.str();
}

FileNotFoundException::FileNotFoundException(const std::string& missingFilename)
    : filename{"File not found: " + missingFilename}
{
}

char const* FileNotFoundException::what() const noexcept
{
    return filename.c_str();
}
}
