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

#include <mutex>

#include "BitFunnel/NonCopyable.h"    // Mutex inherits from NonCopyable.


namespace BitFunnel
{
    // TODO: remove this entire class. This was originally a wrapper for
    // the Win32 CRITICAL_SECTION structure.
    // This is being kept in place as a temporary measure to make porting
    // easier, but the std::mutex documentation states:
    // std::mutex is usually not accessed directly: std::unique_lock and std::lock_guard are used to manage locking in an exception-safe manner. 
    class Mutex : NonCopyable
    {
    public:
        // Construct a mutex.
        Mutex();

        // Destroys the mutex.
        ~Mutex();

        void Lock();

        bool TryLock();

        void Unlock();

    private:
        std::mutex m_mutex;
    };
}
