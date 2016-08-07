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

#include <algorithm>    // std::min(), std::max()
#include <limits>       // infinity()

#include "BitFunnel/Exceptions.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Accumulator
    //
    // Computes count, sum, min, max, mean, and variance of a series of
    // nummeric values.
    //
    //*************************************************************************
    class Accumulator
    {
    public:

        // Constructs an accumulator representing an empty sequence of numbers.
        Accumulator()
          : m_count(0),
            m_sum(0),
            m_sumSquared(0),
            m_min(std::numeric_limits<double>::infinity()),
            m_max(-std::numeric_limits<double>::infinity())
        {
        }


        // Updates the accumulated statistics the reflect the addition of
        // another value.
        template <typename T>
        void Record(T value)
        {
            double v = static_cast<double>(value);
            ++m_count;
            m_sum += v;
            m_sumSquared += v * v;
            m_min = std::min(m_min, v);
            m_max = std::max(m_max, v);
        }


        // Returns the mean or average of all of the values recorded so far.
        double GetMean() const
        {
            if (m_count == 0)
            {
                return 0;
            }
            else
            {
                return m_sum / m_count;
            }
        }


        // Returns the minimum value recorded.
        // Throws if no values have been recorded.
        double GetMin() const
        {
            if (m_count == 0)
            {
                throw RecoverableError("Accumulator::GetMin(): no recorded values.");
            }
            return m_min;
        }


        // Returns the maximum value recorded.
        // Throws if no values have been recorded.
        double GetMax() const
        {
            if (m_count == 0)
            {
                throw RecoverableError("Accumulator::GetMax(): no recorded values.");
            }
            return m_max;
        }


        // Returns the variance of the values recorded.
        double GetVariance() const
        {
            if (m_count < 2)
            {
                return 1.0;
            }
            else
            {
                return (m_sumSquared - m_sum * m_sum / m_count) / (m_count - 1);
            }
        }


        // Returns the number of times the Record() method has been invoked.
        // NOTE: does not return the number of unique values.
        size_t GetCount() const
        {
            return m_count;
        }


    private:
        size_t m_count;
        double m_sum;
        double m_sumSquared;
        double m_min;
        double m_max;
    };
}
