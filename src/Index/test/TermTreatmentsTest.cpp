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

#include <iostream>  // TODO: remove.
#include <math.h>  // TODO: remove.

#include "BitFunnel/Term.h"
#include "TermTreatments.h"

namespace BitFunnel
{
    TEST(TreatmentExperimental, Dummy)
    {
        // TreatmentExperimental::TreatmentExperimental(0.1, 10.0);
        double maxDensity = 0.2;
        const int c_maxRowsPerRank = 6;
        double snr = 10.0;
        double density = 0.1;

        std::vector<int> rowInputs(c_maxRankValue + 1, 0);

        Term::IdfX10 idf = 10;
        double frequency = Term::IdfX10ToFrequency(idf);
        const Rank maxRank = (std::min)(Term::ComputeMaxRank(frequency, maxDensity), static_cast<Rank>(c_maxRankValue));


        auto costRows = Temp(frequency, density, snr, static_cast<int>(maxRank), rowInputs, c_maxRowsPerRank);
        auto rows = costRows.second;

        std::cout << "idf:frequency" << static_cast<unsigned>(idf)
                  << ":" << frequency << std::endl;
        for (Rank rank = 0; rank < rows.size(); ++rank)
        {
            if (rows[rank] != 0)
            {
                std::cout << rank << ":" << rows[rank] << std::endl;
            }
        }
    }
}
