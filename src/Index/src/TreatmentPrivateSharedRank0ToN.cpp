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

#include <algorithm>

#include "BitFunnel/Term.h"
#include "TreatmentPrivateSharedRank0ToN.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0ToN
    //
    // Two rank 0 rows followed by rows of increasing rank until the bit density
    // is > some threshold. Due to limitatons in other BitFunnel code, we also
    // top out at rank 6.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0ToN::TreatmentPrivateSharedRank0ToN(double density,
                                                                   double snr)
    {
        // TODO: what should maxDensity be? Note that this is different from the
        // density liimt that's passed in.
        const double maxDensity = 0.15;
        // Fill up vector of RowConfigurations. GetTreatment() will use the
        // IdfSum() value of the Term as an index into this vector.
        //
        // TODO: we should make sure we get "enough" rows if we end up with a
        // high rank private row.
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            RowConfiguration configuration;

            double frequency = Term::IdfX10ToFrequency(idf);
            if (frequency > density)
            {
                // This term is so common that it must be assigned a private row.
                configuration.push_front(RowConfiguration::Entry(0, 1));
            }
            else
            {
                // TODO: fix other limitations so this can be higher than 6?
                const Rank maxRank = (std::min)(Term::ComputeMaxRank(frequency, maxDensity), static_cast<Rank>(6u));

                int numRows = Term::ComputeRowCount(frequency, density, snr);
                configuration.push_front(RowConfiguration::Entry(0, 2));
                numRows -= 2;
                Rank rank = 1;
                while (rank < maxRank)
                {
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank >= density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1));
                    }
                    ++rank;
                    --numRows;
                }

                double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                if (frequencyAtRank >= density)
                {
                    configuration.push_front(RowConfiguration::Entry(rank,
                                                                     1));
                }
                else
                {
                    if (numRows > 1)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         numRows));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1));
                    }
                }
            }

            m_configurations.push_back(configuration);
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0ToN::GetTreatment(Term::IdfX10 idf) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        idf = std::min(idf, local);
        return m_configurations[idf];
    }
}
