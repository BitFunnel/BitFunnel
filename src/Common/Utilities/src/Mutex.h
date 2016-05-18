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

#include <Windows.h>        // Member of type CRITICAL_SECTION.

#include "BitFunnel/NonCopyable.h"    // Mutex inherits from NonCopyable.


namespace BitFunnel
{
    //*************************************************************************
    //
    // Mutex is a C++ class wrapper for the Win32 CRITICAL_SECTION structure.
    //
    //*************************************************************************
    class Mutex : NonCopyable
    {
    public:
        // Construct a mutex.
        Mutex();

        // Construct a mutex, initializing the critical section with a
        // specified spin count. See Win32 documentation on CRITICAL_SECTION
        // for more information.
        Mutex(unsigned __int32 spinCount);

        // Construct a mutex, initializing the critical section with a
        // specified spin count and flags. See Win32 documentation on
        // CRITICAL_SECTION for more information.
        Mutex(unsigned __int32 spinCount, unsigned __int32 flags);

        // Destroys the mutex.
        ~Mutex();

        // Locks the critical section inside the mutex. This method will
        // block until the lock is successfully acquired.
        void Lock();

        // Attempts to lock the critical section inside the mutex without
        // blocking. Returns true if the critical section was successfully
        // locked. Otherwise returns false.
        bool TryLock();

        // Unlocks the critical section inside the mutex.
        void Unlock();

    private:
        CRITICAL_SECTION m_criticalSection;
    };
}
