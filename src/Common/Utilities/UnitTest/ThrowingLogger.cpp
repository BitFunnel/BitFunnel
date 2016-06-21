#include <stdexcept>

#include "ThrowingLogger.h"


namespace BitFunnel
{
    void ThrowingLogger::Write(char const * /*filename*/,
                               char const * /*function*/,
                               unsigned /*lineNumber*/,
                               Logging::LogLevel /*level*/,
                               char const * /*title*/,
                               char const * /*message*/)
    {
    }


    void ThrowingLogger::Abort()
    {
        throw std::runtime_error("");
    }
}
