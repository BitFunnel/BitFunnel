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

#include <random>   // Typedef for std::mt19937.


namespace BitFunnel
{
    //*************************************************************************
    //
    // RandomInt is a completely self-contained random number generator that
    // uses the Mersenne Twister mt19937 algorithm to generate uniformly
    // distributed integers in a specified range. For more information see
    //      http://en.wikipedia.org/wiki/Mersenne_twister
    //
    // RandomInt is a template class that can be parameterized by any signed or
    // unsigned integer type.
    //
    // RandomInt is not threadsafe.
    //
    //*************************************************************************
    template <typename T>
    class RandomInt
    {
    public:
        RandomInt(unsigned long seed, T minValue, T maxValue);

        T operator()();

    private:
        std::mt19937 m_mt19937Engine;
        std::uniform_int_distribution<T> m_distribution;
    };


    //*************************************************************************
    //
    // RandomReal is a completely self-contained random number generator that
    // uses the Mersenne Twister mt19937 algorithm to generate uniformly
    // distributed floating points values in a specified range. For more
    // information see
    //      http://en.wikipedia.org/wiki/Mersenne_twister
    //
    // RandomReal is a template class that can be parameterized by float or
    // double.
    //
    // RandomReal is not threadsafe.
    //
    //*************************************************************************
    template <typename T>
    class RandomReal
    {
    public:
        RandomReal(unsigned long seed, T minValue, T maxValue);

        T operator()();

    private:
        std::mt19937 m_mt19937Engine;
        std::uniform_real_distribution<T> m_distribution;
    };


    template <typename T>
    RandomInt<T>::RandomInt(unsigned long seed, T minValue, T maxValue)
        : m_mt19937Engine(seed),
          m_distribution(minValue, maxValue)
    {
    }


    template <typename T>
    T RandomInt<T>::operator()()
    {
        return m_distribution(m_mt19937Engine);
    }


    template <typename T>
    RandomReal<T>::RandomReal(unsigned long seed, T minValue, T maxValue)
        : m_mt19937Engine(seed),
          m_distribution(minValue, maxValue)
    {
    }


    template <typename T>
    T RandomReal<T>::operator()()
    {
        return m_distribution(m_mt19937Engine);
    }
}
