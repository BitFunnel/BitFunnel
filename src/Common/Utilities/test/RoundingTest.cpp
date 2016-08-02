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

#include "gtest/gtest.h"

#include "Rounding.h"

namespace BitFunnel
{
    namespace RoundingTest
    {
        const size_t c_testingRange = 100;

        // This test go through all 2^n from n = 1 to n = 63.
        // For each 2^n, it tests the numbers in the following range
        // [2^(n-1), 2^(n-1) + 100],
        // [(2^(n-1) + 2^n) / 2 - 100, (2^(n-1) + 2^n) / 2],
        // [(2^(n-1) + 2^n) / 2, (2^(n-1) + 2^n) / 2 + 100],
        // and [2^n - 100, 2^n],
        TEST(Rounding, RoundToNearestPowerOf2Test)
        {
            const size_t c_one = static_cast<size_t>(1);

            for (size_t i = 1; i < 64; ++i)
            {
                const size_t targetPowerOf2 = c_one << i;
                const size_t previousPowerOf2 = c_one << (i - 1);
                const size_t middleNumber = (targetPowerOf2 + previousPowerOf2)
                    / 2;

                // Test numbers in the first range.
                for (size_t testingNumber = previousPowerOf2;
                     testingNumber <= previousPowerOf2 + c_testingRange
                         && testingNumber <= middleNumber;
                     ++testingNumber)
                {
                    EXPECT_EQ(previousPowerOf2,
                              RoundToTheNearestPowerOf2(testingNumber));
                }

                // Test numbers in the second range.
                size_t secondRangeStart;
                if ((middleNumber - previousPowerOf2) > c_testingRange)
                {
                    secondRangeStart = middleNumber - c_testingRange;
                }
                else
                {
                    secondRangeStart = previousPowerOf2;
                }

                for (size_t testingNumber = secondRangeStart;
                     testingNumber <= middleNumber;
                     ++testingNumber)
                {
                    EXPECT_EQ(previousPowerOf2,
                              RoundToTheNearestPowerOf2(testingNumber));
                }

                // Test numbers in the third range.
                for (size_t testingNumber = middleNumber + 1;
                     testingNumber <= middleNumber + c_testingRange
                         && testingNumber <= targetPowerOf2;
                     ++testingNumber)
                {
                    EXPECT_EQ(targetPowerOf2,
                              RoundToTheNearestPowerOf2(testingNumber));
                }

                // Test numbers in the fourth range.
                size_t fourthRangeStart;
                if ((targetPowerOf2 - middleNumber) > c_testingRange)
                {
                    fourthRangeStart = targetPowerOf2 - c_testingRange;
                }
                else
                {
                    fourthRangeStart = middleNumber + 1;
                }

                for (size_t testingNumber = fourthRangeStart;
                     testingNumber <= targetPowerOf2;
                     ++testingNumber)
                {
                    EXPECT_EQ(targetPowerOf2,
                              RoundToTheNearestPowerOf2(testingNumber));
                }
            }
        }
    }
}
