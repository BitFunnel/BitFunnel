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

#include <cstring>      // memset().

#include "BitFunnel/BitFunnelTypes.h"
#include "CacheLineRecorder.h"
#include "Rounding.h"


namespace BitFunnel
{
    //
    // Lookup table from
    //     https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
    //
    static const uint8_t g_bitsSetTable256[256] =
    {
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
        B6(0), B6(1), B6(1), B6(2)
    };


    CacheLineRecorder::CacheLineRecorder(size_t sliceBufferSize)
      : m_sliceBufferSize(sliceBufferSize),
        m_base(nullptr)
    {
        size_t cacheLineCount =
            RoundUp(sliceBufferSize, c_bytesPerCacheLine) / c_bytesPerCacheLine;

        m_bitArraySize =
            RoundUp(cacheLineCount, c_bitsPerByte) / c_bitsPerByte;

        m_bitArray.reset(new uint8_t[m_bitArraySize]);
    }


    void CacheLineRecorder::SetBase(void const * base)
    {
        m_base = reinterpret_cast<char const *>(base);
    }


    void CacheLineRecorder::RecordAccess(void const * ptr)
    {
        const ptrdiff_t offset = static_cast<char const *>(ptr) - m_base;
        const size_t cacheLineNumber = offset / c_bytesPerCacheLine;
        const size_t byteIndex = cacheLineNumber / c_bitsPerByte;
        const uint8_t bitMask = 1ull << (cacheLineNumber % c_bitsPerByte);
        m_bitArray[byteIndex] |= bitMask;
    }


    size_t CacheLineRecorder::GetCacheLinesAccessed() const
    {
        size_t count = 0;
        for (size_t i = 0; i < m_bitArraySize; ++i)
        {
            count += g_bitsSetTable256[m_bitArray[i]];
        }
        return count;
    }


    void CacheLineRecorder::Reset()
    {
        memset(m_bitArray.get(), 0ull, m_bitArraySize);
    }
}
