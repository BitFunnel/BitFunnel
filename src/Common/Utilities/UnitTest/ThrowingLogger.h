#pragma once

#include "LoggerInterfaces/ILogger.h"


namespace BitFunnel
{

    //*************************************************************************
    //
    // An ILogger that throws a runtime_error instead of calling abort()
    //
    //*************************************************************************
    class ThrowingLogger : public Logging::ILogger
    {
    public:
        void Write(char const * filename,
                   char const * function,
                   unsigned lineNumber,
                   Logging::LogLevel level,
                   char const * title,
                   char const * message);

        void Abort();
    };
}
