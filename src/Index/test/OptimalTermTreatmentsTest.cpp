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

#include <cmath>
#include <iostream>

#include "OptimalTermTreatments.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    TEST(OptimalTermTreatmentsTest, Placeholder)
    {
        // This test exists solely as a demonstration of the optimal term
        // treatment algorithm. It is currently disabled so that it doesn't
        // slow down the unit test suite.
        // OptimalTermTreatments();
    }


    double ProbNotZero(double density)
    {
        return 1.0 - pow(1 - density, 64);
    }


    TEST(OptimalTermTreatmentsTest, Analyzer)
    {
        const double c_density = 0.1;
        const double c_signal = 0.00125893;
        const double c_debug_signal = Term::FrequencyAtRank(c_signal, 5);
        std::vector<int> rows = {2, 0, 0, 0, 0, 1};
        auto metrics0 = AnalyzeAlternate(rows, c_density, c_signal);
        size_t rowConfig = SizeTFromRowVector(rows);
        auto metrics1 = Analyze(rowConfig, c_density, c_signal, false);

        double c0 = metrics0.GetQuadwords();
        double c1 = metrics1.second.GetQuadwords();


        // Manual computation for AnalyzeAlternate formulation.
        double initialNoise = c_density - c_debug_signal;
        double noiseAtZero = c_density - c_signal;

        double rankDownSignalDelta = c_debug_signal - c_signal;
        double noiseAfterR5R0 = (rankDownSignalDelta + initialNoise) * noiseAtZero;
        double noiseAfterR5R0R0 = noiseAfterR5R0 * noiseAtZero;

        double initialDensity = c_density;
        double densityAfterR5R0 = c_signal + noiseAfterR5R0;
        double densityAfterR5R0R0 = c_signal + noiseAfterR5R0R0;

        double initialPNonZero = ProbNotZero(initialDensity);
        double PNonZeroAfterR5R0 = ProbNotZero(densityAfterR5R0);
        double PNonZeroAfterR5R0R0 = ProbNotZero(densityAfterR5R0R0);

        std::cout << "==============================" << std::endl;
        std::cout << "signal:debug_signal " << c_signal << ":" << c_debug_signal << std::endl;
        std::cout << "qword0:qword1 " << c0 << ":" << c1 << std::endl;
        std::cout << "noise0:noise1:noise2 "
                  << initialNoise << ":"
                  << noiseAfterR5R0 << ":"
                  << noiseAfterR5R0R0 << std::endl;

        std::cout << "density0:density1:density2 "
                  << initialDensity << ":"
                  << densityAfterR5R0 << ":"
                  << densityAfterR5R0R0 << std::endl;

        std::cout << "P0:P1:P2 "
                  << initialPNonZero << ":"
                  << PNonZeroAfterR5R0 << ":"
                  << PNonZeroAfterR5R0R0 << std::endl;


        std::cout << "ExpectedQwords: " << (1.0 / (1 << 5))
            + initialPNonZero + PNonZeroAfterR5R0 << std::endl;


        ASSERT_FALSE(std::isinf(c0));
        ASSERT_FALSE(std::isinf(c1));
        ASSERT_FALSE(std::isnan(c0));
        ASSERT_FALSE(std::isnan(c1));
        ASSERT_LE(std::abs(c0-c1), 0.005);
    }


    TEST(OptimalTermTreatmentsTest, Analyze)
    {
        double density = 0.1;
        double snr = 10;

	std::cout << "================================================================ Rank0" << std::endl;
        TreatmentPrivateSharedRank0 t0(density, snr);
        AnalyzeTermTreatment(t0, density);

	std::cout << "================================================================ Rank0And3" << std::endl;
        TreatmentPrivateSharedRank0And3 t1(density, snr);
        AnalyzeTermTreatment(t1, density);

	std::cout << "================================================================ Experimental" << std::endl;
        TreatmentExperimental t2(density, snr);
        AnalyzeTermTreatment(t2, density);

	std::cout << "================================================================ Optimal" << std::endl;
        TreatmentOptimal t3(density, snr);
        AnalyzeTermTreatment(t3, density);
    }
}
