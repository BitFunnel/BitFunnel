#include "stdafx.h"

#include "LoggerInterfaces/Logging.h"
#include "Rounding.h"
#include "SuiteCpp/UnitTest.h"

namespace BitFunnel
{
    namespace RoundingUnitTest
    {
        const size_t c_testingRange = 100;

        // This test go through all 2^n from n = 1 to n = 63.
        // For each 2^n, it tests the numbers in the following range
        // [2^(n-1), 2^(n-1) + 100], 
        // [(2^(n-1) + 2^n) / 2 - 100, (2^(n-1) + 2^n) / 2],
        // [(2^(n-1) + 2^n) / 2, (2^(n-1) + 2^n) / 2 + 100],
        // and [2^n - 100, 2^n],
        TestCase(RoundToNearestPowerOf2Test)
        {
            const size_t c_one = static_cast<size_t>(1);

            for (size_t i = 1; i < 64; ++i)
            {
                const size_t targetPowerOf2 = c_one << i;
                const size_t previousPowerOf2 = c_one << (i - 1);
                const size_t middleNumber = (targetPowerOf2 + previousPowerOf2) / 2;

                // Test numbers in the first range.
                for (size_t testingNumber = previousPowerOf2; 
                     testingNumber <= previousPowerOf2 + c_testingRange && testingNumber <= middleNumber;
                     ++testingNumber)
                {
                    TestEqual(previousPowerOf2, RoundToTheNearestPowerOf2(testingNumber));
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
                    TestEqual(previousPowerOf2, RoundToTheNearestPowerOf2(testingNumber));
                }
                
                // Test numbers in the third range.
                for (size_t testingNumber = middleNumber + 1; 
                     testingNumber <= middleNumber + c_testingRange && testingNumber <= targetPowerOf2;
                     ++testingNumber)
                {
                    TestEqual(targetPowerOf2, RoundToTheNearestPowerOf2(testingNumber));
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
                    TestEqual(targetPowerOf2, RoundToTheNearestPowerOf2(testingNumber));
                }
            }
        }
    }
}
