// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "LoggerInterfaces/LogLevel.h"


// Macro that logs a message. The macro gathers the source file name and the
// line number. The caller may specify optional arguments that will be passed to
// sprintf() to create the rest of the log message.
#define LogB(level, title, format, ...)      \
    Logging::LogImpl(__FILE__,        \
                     __FUNCTION__,    \
                     __LINE__,        \
                     level,           \
                     title,           \
                     format,          \
                     __VA_ARGS__);


// Macro that logs a message and aborts the program when a condition expression
// evaluates to false. The caller may specify optional arguments that will be
// passed to sprintf() to create the rest of the log message.
#define LogAssertB(condition, ...)                          \
{                                                           \
    if (!(condition))                                       \
    {                                                       \
        Logging::LogAbortImpl(__FILE__,                     \
                              __FUNCTION__,                 \
                              __LINE__,                     \
                              "LogAssert("#condition")",    \
                              __VA_ARGS__);                 \
    }                                                       \
}


// Macro that logs a message and aborts the program. The caller may specify
// optional arguments that will be passed to sprintf() to create the rest of
// the log message.

#define LogAbortB(...)                     \
    Logging::LogAbortImpl(__FILE__,        \
                          __FUNCTION__,    \
                          __LINE__,        \
                          "LogAbort()",    \
                          __VA_ARGS__);


namespace Logging
{
    class ILogger;

    // Sets the current logger. Use NULL to prevent logging.
    // This method is thread safe.
    void RegisterLogger(ILogger * logger);


    // Generates a log message for a specified title, source file, function,
    // and line number.
    // The format arument and those that follow are passed to sprintf() to
    // create the log message.
    // This method is thread safe.
    void LogImpl(char const * filename,
             char const * function,
             unsigned lineNumber,
             LogLevel level,
             char const * title,
             char const * format,
             ...);


    // Logs a message and then instructs the logger to terminate the
    // program. The condition parameter has the text of the condition
    // causing the program to shut down. Its value is typically set with
    // the text of the first argument to the LogAssert() macro. Any
    // arguments after condition are passed to sprintf() to create the rest
    // of the log message.
    // This method is thread safe.
    void LogAbortImpl(char const * filename,
                      char const * function,
                      unsigned lineNumber,
                      char const * condition);
    void LogAbortImpl(char const * filename,
                      char const * function,
                      unsigned lineNumber,
                      char const * condition,
                      char const * format,
                      ...);
}
