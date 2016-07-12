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


#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/PackedTermInfo.h"


namespace BitFunnel
{
    PackedTermInfo::PackedTermInfo(unsigned rowIdStart, unsigned rowIdCount)
        : m_rowIdStart(rowIdStart),
          m_rowIdCount(static_cast<const uint8_t>(rowIdCount))
    {
        // Check for overflow in cast to uint8_t.
        LogAssertB(m_rowIdCount == rowIdCount, "overflow in cast to uint8_t");
    }


    PackedTermInfo::PackedTermInfo()
        : m_rowIdStart(0),
          m_rowIdCount(0)
    {
    }


    bool PackedTermInfo::IsEmpty() const
    {
        return (m_rowIdCount == 0);
    }


    size_t PackedTermInfo::GetRowIdStart() const
    {
        return m_rowIdStart;
    }


    unsigned PackedTermInfo::GetRowIdCount() const
    {
        return m_rowIdCount;
    }


    bool PackedTermInfo::operator==(const PackedTermInfo& rhs) const
    {
        return (m_rowIdStart == rhs.m_rowIdStart)
               && (m_rowIdCount == rhs.m_rowIdCount);
    }
}
