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

#include <cstdarg>
#include <cstring>
#include <sstream>

#include "LoggerInterfaces/ILogger.h"
#include "LoggerInterfaces/Logging.h"
#include "NullLogger.h"

#ifdef _MSC_VER
#define COUNT_OF _countof
#else
template <typename T, size_t N>
char ( &_ArraySizeHelper( T (&array)[N] ))[N];
#define COUNT_OF( array ) (sizeof( _ArraySizeHelper( array ) ))
#endif

namespace Logging
{
    const unsigned c_messageBufferSize = 1024;

    // A marker to be placed at the end of the truncated log lines.
    constexpr char c_truncatedMessage[] = "[TEXT TRUNCATED]";
    static_assert(c_messageBufferSize > COUNT_OF(c_truncatedMessage),
                  "The message buffer is too short for truncation marker");

    static NullLogger g_nullLogger;

    // It is not possible for this to be NULL
    static ILogger * g_logger = &g_nullLogger;


    void RegisterLogger(ILogger * logger)
    {
        g_logger = (logger != NULL) ? logger : &g_nullLogger;
    }


    void LogImpl(char const * filename,
                  char const * function,
                  unsigned lineNumber,
                  LogLevel level,
                  char const * title,
                  char const * format,
                  ...)
    {
        char message[c_messageBufferSize];

        va_list arguments;
        va_start(arguments, format);
#ifdef _MSC_VER
        const int result = vsnprintf_s(message, _TRUNCATE, format, arguments);
#else
        const int result = vsnprintf(&message[0], c_messageBufferSize, format, arguments);
#endif

        va_end(arguments);

        // If truncation occurred, place the truncation marker at the end of the
        // buffer. Note that COUNT_OF also counts the terminating '\0'.
        if (result == -1)
        {
            const unsigned position = COUNT_OF(message) - COUNT_OF(c_truncatedMessage);
            memcpy(&message[position], &c_truncatedMessage[0], COUNT_OF(c_truncatedMessage));
        }

        g_logger->Write(filename, function, lineNumber, level, title, message);
    }


    void LogAbortImpl(char const * filename,
                      char const * function,
                      unsigned lineNumber,
                      char const * condition)
    {
        std::stringstream sstream;
        sstream << condition << " failed";

        g_logger->Write(filename, function, lineNumber, Assert, "LogAbort", sstream.str().c_str());
        g_logger->Abort();
    }


    void LogAbortImpl(char const * filename,
                      char const * function,
                      unsigned lineNumber,
                      char const * condition,
                      char const * format,
                      ...)
    {
        char message[c_messageBufferSize];
        va_list arguments;
        va_start(arguments, format);

#ifdef _MSC_VER
        vsprintf_s(message, sizeof(message), format, arguments);
#else
        vsprintf(message, format, arguments);
#endif
        va_end(arguments);

        std::stringstream sstream;
        sstream << condition << " failed: " << message;

        g_logger->Write(filename, function, lineNumber, Assert, "LogAbort", sstream.str().c_str());
        g_logger->Abort();
    }
}

#undef COUNT_OF
