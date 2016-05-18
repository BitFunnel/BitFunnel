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

#include <Windows.h>        // For LARGE_INTEGER

namespace BitFunnel
{
    class Stopwatch
    {
    public:
        // Constructs Stopwatch() and records the start time.
        Stopwatch();

        // Resets the start time to the current time.
        void Reset();

        // Returns the elapsed time in seconds since the Stopwatch
        // was constructed or Reset() was last called.
        double ElapsedTime() const;

    private:
        // Given a start time, end time, and timer frequency, compute and return
        // the elapsed time in seconds.
        static double ComputeElapsedTime(LARGE_INTEGER begin, LARGE_INTEGER end, LARGE_INTEGER frequency);

        // Timer frequency in ticks per second.
        LARGE_INTEGER m_frequency;

        // Time Stopwatch was last reset (expressed as a tick count).
        LARGE_INTEGER m_start;
    };
}

