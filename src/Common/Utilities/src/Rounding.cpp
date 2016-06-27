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


#include "Rounding.h"


namespace BitFunnel
{
    size_t RoundUpPowerOf2(size_t valueToBeRounded)
    {
        if (0 == valueToBeRounded)
        {
            return 1;
        }

        size_t value = valueToBeRounded;

        // The source of this algorithm is
        // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;
        value++;

        return value;
    }


    size_t RoundToTheNearestPowerOf2(size_t valueToBeRounded)
    {
        const size_t nextPowerOf2 = RoundUpPowerOf2(valueToBeRounded);
        const size_t previousPowerOf2 = nextPowerOf2 >> 1;

        return (nextPowerOf2 - valueToBeRounded) >=
            (valueToBeRounded - previousPowerOf2)
               ? previousPowerOf2
               : nextPowerOf2;
    }


    size_t RoundUp(size_t requestedSize, size_t roundUpAlignment)
    {
        const size_t roundedValue = (requestedSize + roundUpAlignment - 1)
                                    / roundUpAlignment * roundUpAlignment;

        return roundedValue;
    }
}
