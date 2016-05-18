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

#include <Windows.h>

#include "ThreadsafeCounter.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // ThreadsafeCounter32
    //
    //*************************************************************************
    ThreadsafeCounter32::ThreadsafeCounter32(unsigned __int32 initialValue)
        : m_value(initialValue)
    {
    }


    ThreadsafeCounter32::ThreadsafeCounter32(const ThreadsafeCounter32& other)
    {
        ThreadsafeSetValue(other.ThreadsafeGetValue());
    }


    unsigned __int32 ThreadsafeCounter32::ThreadsafeIncrement()
    {
        return InterlockedIncrement(&m_value);
    }


    bool ThreadsafeCounter32::TryThreadsafeBoundedIncrement(unsigned __int32& newValue, unsigned __int32 threshold)
    {
        for (;;)
        {
            long current = m_value;
            // TODO: REVIEW: Is this cast to long ok? Should we check for overflow?
            if (current + 1 > static_cast<long>(threshold))
            {
                return false;
            }

            if (current == InterlockedCompareExchange(&m_value, current + 1, current))
            {
                newValue = static_cast<unsigned __int32>(current + 1);
                return true;
            }
        }
    }


    unsigned __int32 ThreadsafeCounter32::ThreadsafeDecrement()
    {
        return InterlockedDecrement(&m_value);
    }


    unsigned __int32 ThreadsafeCounter32::ThreadsafeAdd(unsigned __int32 value)
    {
        return InterlockedAdd(&m_value, value);
    }


    unsigned __int32 ThreadsafeCounter32::ThreadsafeAdd(__int32 value)
    {
        return InterlockedAdd(&m_value, value);
    }


    unsigned __int32 ThreadsafeCounter32::ThreadsafeSetValue(unsigned __int32 value)
    {
        return InterlockedExchange(&m_value, value);
    }


    unsigned __int32 ThreadsafeCounter32::ThreadsafeGetValue() const
    {
        // DESIGN NOTE: Uses interlocked compare exchange for read to enforce
        // a memory barrier.
        return InterlockedCompareExchange(&m_value, 0, 0);
    }


    ThreadsafeCounter32& ThreadsafeCounter32::operator=(const ThreadsafeCounter32& other)
    {
        ThreadsafeSetValue(other.ThreadsafeGetValue());
        return *this;
    }


    //*************************************************************************
    //
    // ThreadsafeCounter64
    //
    //*************************************************************************
    ThreadsafeCounter64::ThreadsafeCounter64(unsigned __int64 initialValue)
        : m_value(initialValue)
    {
    }


    ThreadsafeCounter64::ThreadsafeCounter64(const ThreadsafeCounter64& other)
    {
        ThreadsafeSetValue(other.ThreadsafeGetValue());
    }


    unsigned __int64 ThreadsafeCounter64::ThreadsafeIncrement()
    {
        return InterlockedIncrement64(&m_value);
    }


    bool ThreadsafeCounter64::TryThreadsafeBoundedIncrement(unsigned __int64& newValue, unsigned __int64 threshold)
    {
        for (;;)
        {
            long long current = m_value;
            // TODO: REVIEW: Is this cast to long long ok? Should we check for overflow?
            if (current + 1 > static_cast<long long>(threshold))
            {
                return false;
            }

            if (current == InterlockedCompareExchange64(&m_value, current + 1, current))
            {
                newValue = static_cast<unsigned __int64>(current + 1);
                return true;
            }
        }
    }


    unsigned __int64 ThreadsafeCounter64::ThreadsafeDecrement()
    {
        return InterlockedDecrement64(&m_value);
    }


    unsigned __int64 ThreadsafeCounter64::ThreadsafeAdd(unsigned __int64 value)
    {
        return InterlockedAdd64(&m_value, value);
    }


    unsigned __int64 ThreadsafeCounter64::ThreadsafeAdd(__int64 value)
    {
        return InterlockedAdd64(&m_value, value);
    }


    unsigned __int64 ThreadsafeCounter64::ThreadsafeSetValue(unsigned __int64 value)
    {
        return InterlockedExchange64(&m_value, value);
    }


    unsigned __int64 ThreadsafeCounter64::ThreadsafeGetValue() const
    {
        // DESIGN NOTE: Uses interlocked compare exchange for read to enforce
        // a memory barrier.
        return InterlockedCompareExchange64(&m_value, 0, 0);
    }


    ThreadsafeCounter64& ThreadsafeCounter64::operator=(const ThreadsafeCounter64& other)
    {
        ThreadsafeSetValue(other.ThreadsafeGetValue());
        return *this;
    }
}
