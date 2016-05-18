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

#include "Stopwatch.h"


namespace BitFunnel
{
    Stopwatch::Stopwatch()
    {   
        // Note: QueryPerformanceFrequency/Counter calls cannot fail since WindowsXP.
        QueryPerformanceFrequency(&m_frequency);
        Reset();
    }


    void Stopwatch::Reset()
    {
        QueryPerformanceCounter(&m_start);
    }


    double Stopwatch::ElapsedTime() const
    {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);

        return ComputeElapsedTime(m_start, end, m_frequency);
    }


    double Stopwatch::ComputeElapsedTime(LARGE_INTEGER begin, LARGE_INTEGER end, LARGE_INTEGER frequency)
    {
        return (double)(end.QuadPart - begin.QuadPart) / (double)frequency.QuadPart;
    }
}
