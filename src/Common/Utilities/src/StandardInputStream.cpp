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


#include <istream>
#include <limits>

#include "BitFunnel/Utilities/StandardInputStream.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    StandardInputStream::StandardInputStream(std::istream& stream)
        : m_stream(stream)
    {
    }


    size_t StandardInputStream::Read(char* destination, size_t byteCount)
    {
        // This check and cast of byteCount is because stream.write takes a
        // std::streamsize, and std::streamsize is signed. There should be no
        // reasonable way to overflow this, but we check anyway since this is
        // not believed to be a performance critical path. It's possible that we
        // should just use streamsize everywhere.
        LogAssertB(byteCount < static_cast<size_t>(std::numeric_limits<std::streamsize>::max()),
                   "streamsize overflow.");
        m_stream.read(destination, static_cast<std::streamsize>(byteCount));

        LogAssertB(m_stream.gcount() >= 0,
                   "gcount underflow.");
        return static_cast<size_t>(m_stream.gcount());
    }
}
