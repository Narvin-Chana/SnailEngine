#pragma once

#include <string>
#include <exception>
#include <limits>

namespace Snail
{

/**
 * \brief Generic exception class for Snail.
 * Should be extended to provide for clearer exceptions.
 */
class SnailException : public std::exception
{
public:
    SnailException() noexcept = default;
    SnailException(unsigned int line, const char* file) noexcept;
    SnailException(const char* errorMessage) noexcept;
    const char* what() const noexcept override;

    virtual const char* GetType() const noexcept;
    unsigned int GetLine() const noexcept;
    const std::string& GetFile() const noexcept;
    std::string GetOriginString() const noexcept;

private:
    unsigned int line = std::numeric_limits<unsigned int>::max();
    std::string file = "Not defined.";

protected:
    mutable std::string whatBuffer;
};

class FileNotFoundException : public SnailException
{
    std::string filename;
public:
    FileNotFoundException(const std::string& missingFilename);
    char const* what() const noexcept override;
};

}

