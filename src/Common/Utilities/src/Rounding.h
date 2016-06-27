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

#include <stddef.h>  // For size_t.

namespace BitFunnel
{
    // Get the nearest power of 2 integer for valueToBeRounded.
    // For example, for 17, it should return 16, and for 30, it should return 32.
    size_t RoundToTheNearestPowerOf2(size_t valueToBeRounded);

    // Get the nearest power of 2 integer for valueToBeRounded which is greater than
    // valueToBeRounded.
    // For example, for 17, it should return 32, and for 30, it should return 32.
    size_t RoundUpPowerOf2(size_t valueToBeRounded);

    // Rounds up the requested size to the next multiple of
    // roundUpAlignment.
    size_t RoundUp(size_t requestedSize, size_t roundUpAlignment);
}
