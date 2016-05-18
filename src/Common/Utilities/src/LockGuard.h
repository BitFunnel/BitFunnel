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

#include "BitFunnel/NonCopyable.h"    // LockGuard inherits from NonCopyable.


namespace BitFunnel
{
    class Mutex;

    //*************************************************************************
    //
    // LockGuard is an RAII helper class that locks a mutex on construction and
    // unlocks the mutex on destruction. LockGuard is designed to be used to
    // take a lock for the remainder of a C++ block scope.
    // There are two flavors of the LockGuard
    // 1. With infinite timeout - LockGuard will block on trying to acquire a 
    //    lock and won't exit until it has been taken.
    // 2. Throwing LockGuard - tries to acquire a lock without waiting and 
    //    throws if the lock could not be acquired.
    //
    //*************************************************************************
    class LockGuard : NonCopyable
    {
    public:
        // Construct a LockGuard that immediately locks the Mutex parameter.
        // If throwIfNotImmediatelyAcquired = false, this will block until the
        // lock has been acquired.
        // If throwIfNotImmediatelyAcquired = true and the Mutex is not 
        // immediately acquired, this function throws.
        explicit LockGuard(Mutex& lock, 
                           bool throwIfNotImmediatelyAcquired = false);

        // Unlocks the mutex and destroys the LockGuard.
        ~LockGuard();

    private:
        Mutex& m_mutex;
    };
}
