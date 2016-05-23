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


#include <atomic>

#include <inttypes.h>
#include <stddef.h>


// TODO: remove this?
// This used to use Windows specific primitives, but was replaced by
// std::atomic.
// This is being kept for now to make porting easier.

namespace BitFunnel
{
    //*************************************************************************
    //
    // ThreadsafeCounter32 provides a uint32_t variable that can be
    // used as a counter accessed by multiple threads. All methods utilize full
    // memory barriers and the increment and decrement methods use the Windows
    // Interlocked functions.
    //
    // The main purpose of ThreadsafeCounter32 is to isolate in a single class,
    // all of the details of volatile, memory alignment, and memory barriers.
    // This allows us to change the implementation as we get more knowledge of
    // the implementation of barriers and if we move to a different processor
    // architecture.
    //
    // ThreadsafeCounter32, ThreadsafeCounter64, and ThreadsafeFlag should
    // eliminate the need for the volatile keyword elsewhere in our codebase.
    //
    //*************************************************************************
    class ThreadsafeCounter32
    {
    public:
        // Initialize count to a specified value with full memory barrier.
        // Constructor is explicit make the use of ThreadsafeCounter32 more
        // apparent in the code. With a std::vector<ThreadsafeCount>, one may
        // attempt to write "v[i] = 5;". Adding explicit forces the user to
        // write "v[i] = ThreadsafeCounter32(5);".
        // WARNING: users of ThreadsafeCounter32 rely on the default
        // constructor behavior that initializes the counter to zero. The
        // main scenario is array initialization.
        explicit ThreadsafeCounter32(uint32_t initialValue = 0);

        // Copy constructor with full memory barrier.
        ThreadsafeCounter32(const ThreadsafeCounter32& other);

        // Increments counter and returns the resulting value.
        uint32_t ThreadsafeIncrement();

        // Attempts to atomically increment the counter if its post-increment
        // value will not exceed a specified threshold value. If successful,
        // reference parameter value will contain the post-increment value and
        // the function will return true.
        bool TryThreadsafeBoundedIncrement(uint32_t& value,
                                           uint32_t threshold);

        // Decrements counter and returns the resulting value.
        uint32_t ThreadsafeDecrement();

        // Adds an unsigned value and returns the result
        uint32_t ThreadsafeAdd(uint32_t value);

        // Adds a signed value and returns the result
        uint32_t ThreadsafeAdd(int32_t value);

        // Sets the counter to a specified value. Returns the previous value.
        uint32_t ThreadsafeSetValue(uint32_t value);

        // Returns the current counter value.
        uint32_t ThreadsafeGetValue() const;

        // Assignment operator uses ThreadsafeSetValue to enforce memory
        // barrier.
        ThreadsafeCounter32& operator=(const ThreadsafeCounter32& other);

    private:
        // TODO: mark mutable?
        // TODO: mark aligned?
        std::atomic<uint32_t> m_value;
    };


    //*************************************************************************
    //
    // ThreadsafeCounter64 provides a uint64_t variable that can be
    // used as a counter accessed by multiple threads. All methods utilize full
    // memory barriers and the increment and decrement methods use the Windows
    // Interlocked functions.
    //
    // The main purpose of ThreadsafeCounter64 is to isolate in a single class,
    // all of the details of volatile, memory alignment, and memory barriers.
    // This allows us to change the implementation as we get more knowledge of
    // the implementation of barriers and if we move to a different processor
    // architecture.
    //
    // ThreadsafeCounter32, ThreadsafeCounter64, and ThreadsafeFlag should
    // eliminate the need for the volatile keyword elsewhere in our codebase.
    //
    //*************************************************************************
    class ThreadsafeCounter64
    {
    public:
        // Initialize count to a specified value with full memory barrier.
        // Constructor is explicit make the use of ThreadsafeCounter64 more
        // apparent in the code. With a std::vector<ThreadsafeCount>, one may
        // attempt to write "v[i] = 5;". Adding explicit forces the user to
        // write "v[i] = ThreadsafeCounter64(5);".
        // WARNING: users of ThreadsafeCounter32 rely on the default
        // constructor behavior that initializes the counter to zero. The
        // main scenario is array initialization.
        explicit ThreadsafeCounter64(uint64_t initialValue = 0);

        // Copy constructor with full memory barrier.
        ThreadsafeCounter64(const ThreadsafeCounter64& other);

        // Increments counter and returns the resulting value.
        uint64_t ThreadsafeIncrement();

        // Attempts to atomically increment the counter if its post-increment
        // value will not exceed a specified threshold value. If successful,
        // reference parameter value will contain the post-increment value and
        // the function will return true.
        bool TryThreadsafeBoundedIncrement(uint64_t& value,
                                           uint64_t threshold);


        // Decrements counter and returns the resulting value.
        uint64_t ThreadsafeDecrement();

        // Adds an unsigned value and returns the result
        uint64_t ThreadsafeAdd(uint64_t value);

        // Adds a signed value and returns the result
        uint64_t ThreadsafeAdd(int64_t value);

        // Sets the counter to a specified value. Returns the previous value.
        uint64_t ThreadsafeSetValue(uint64_t value);

        // Returns the current counter value.
        uint64_t ThreadsafeGetValue() const;

        // Assignment operator uses ThreadsafeSetValue to enforce memory
        // barrier.
        ThreadsafeCounter64& operator=(const ThreadsafeCounter64& other);

    private:
        // TODO: mark mutable?
        // TODO: mark aligned?
        std::atomic<uint64_t> m_value;
    };
}
