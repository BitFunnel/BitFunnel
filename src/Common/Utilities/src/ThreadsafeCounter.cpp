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

#include "ThreadsafeCounter.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // ThreadsafeCounter32
    //
    //*************************************************************************
    ThreadsafeCounter32::ThreadsafeCounter32(uint32_t initialValue)
        : m_value(initialValue)
    {
    }


    ThreadsafeCounter32::ThreadsafeCounter32(const ThreadsafeCounter32& other)
    {
        m_value = other.ThreadsafeGetValue();
    }


    uint32_t ThreadsafeCounter32::ThreadsafeIncrement()
    {
        return ++m_value;
    }


    bool ThreadsafeCounter32::TryThreadsafeBoundedIncrement(uint32_t& newValue, uint32_t threshold)
    {
        for (;;)
        {
            // TODO: check for overflow?
            uint32_t current = m_value;
            if (current + 1 > static_cast<long>(threshold))
            {
                return false;
            }

            if (m_value.compare_exchange_weak(current, current + 1))
            {
                newValue = static_cast<uint32_t>(current + 1);
                return true;
            }
        }
    }


    uint32_t ThreadsafeCounter32::ThreadsafeDecrement()
    {
        return --m_value;
    }


    uint32_t ThreadsafeCounter32::ThreadsafeAdd(uint32_t value)
    {
        return m_value += value;
    }


    uint32_t ThreadsafeCounter32::ThreadsafeAdd(int32_t value)
    {
        return m_value += value;
    }


    uint32_t ThreadsafeCounter32::ThreadsafeSetValue(uint32_t value)
    {
        return m_value = value;
    }


    uint32_t ThreadsafeCounter32::ThreadsafeGetValue() const
    {
        return m_value;
    }


    ThreadsafeCounter32& ThreadsafeCounter32::operator=(const ThreadsafeCounter32& other)
    {
        m_value = other.ThreadsafeGetValue();
        return *this;
    }


    //*************************************************************************
    //
    // ThreadsafeCounter64
    //
    //*************************************************************************
    ThreadsafeCounter64::ThreadsafeCounter64(uint64_t initialValue)
        : m_value(initialValue)
    {
    }


    ThreadsafeCounter64::ThreadsafeCounter64(const ThreadsafeCounter64& other)
    {
        m_value = other.ThreadsafeGetValue();
    }


    uint64_t ThreadsafeCounter64::ThreadsafeIncrement()
    {
        return ++m_value;
    }


    bool ThreadsafeCounter64::TryThreadsafeBoundedIncrement(uint64_t& newValue, uint64_t threshold)
    {
        for (;;)
        {
            // TODO: check for overflow?
            uint64_t current = m_value;
            if (current + 1 > threshold)
            {
                return false;
            }

            if (m_value.compare_exchange_weak(current, current + 1))
            {
                newValue = static_cast<uint64_t>(current + 1);
                return true;
            }
        }
    }


    uint64_t ThreadsafeCounter64::ThreadsafeDecrement()
    {
        return --m_value;
    }


    uint64_t ThreadsafeCounter64::ThreadsafeAdd(uint64_t value)
    {
        return m_value += value;
    }


    uint64_t ThreadsafeCounter64::ThreadsafeAdd(int64_t value)
    {
        return m_value += value;
    }


    uint64_t ThreadsafeCounter64::ThreadsafeSetValue(uint64_t value)
    {
        return m_value = value;
    }


    uint64_t ThreadsafeCounter64::ThreadsafeGetValue() const
    {
        return m_value;
    }


    ThreadsafeCounter64& ThreadsafeCounter64::operator=(const ThreadsafeCounter64& other)
    {
        m_value = other.ThreadsafeGetValue();
        return *this;
    }
}
