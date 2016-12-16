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

#include "OptimalTermTreatments.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    TEST(OptimalTermTreatmentsTest, Placeholder)
    {
        // This test exists solely as a demonstration of the optimal term
        // treatment algorithm. It is currently disabled so that it doesn't
        // slow down the unit test suite.
        OptimalTermTreatments();
    }


    TEST(OptimalTermTreatmentsTest, Analyze)
    {
        double density = 0.1;
        double snr = 10;

        TreatmentPrivateSharedRank0And3 t1(density, snr);
        AnalyzeTermTreatment(t1, density);

        TreatmentExperimental t2(density, snr);
        AnalyzeTermTreatment(t2, density);
    }
}
