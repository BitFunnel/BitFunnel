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
#include <iostream>             // TODO: Remove this temporary include.
#include <limits>  // NaN.
#include <math.h>

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Term.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************
    std::unique_ptr<ITermTreatment> Factories::CreateTreatmentPrivateRank0()
    {
        return std::unique_ptr<ITermTreatment>(new TreatmentPrivateRank0());
    }


    std::unique_ptr<ITermTreatment>
        Factories::CreateTreatmentPrivateSharedRank0(double density,
                                                     double snr)
    {
        return
            std::unique_ptr<ITermTreatment>(
                new TreatmentPrivateSharedRank0(density, snr));
    }


    std::unique_ptr<ITermTreatment>
        Factories::CreateTreatmentPrivateSharedRank0And3(double density,
                                                        double snr)
    {
        return
            std::unique_ptr<ITermTreatment>(
                new TreatmentPrivateSharedRank0And3(density, snr));
    }


    std::unique_ptr<ITermTreatment>
        Factories::CreateTreatmentPrivateSharedRank0ToN(double density,
                                                        double snr)
    {
        return
            std::unique_ptr<ITermTreatment>(
                new TreatmentPrivateSharedRank0ToN(density, snr));
    }



    std::unique_ptr<ITermTreatment>
        Factories::CreateTreatmentExperimental(double density,
                                               double snr)
    {
        return
            std::unique_ptr<ITermTreatment>(
                new TreatmentExperimental(density, snr));
    }


    //*************************************************************************
    //
    // TreatmentPrivateRank0
    //
    // All terms get the same treatment - a single, private, rank 0 row.
    //
    //*************************************************************************
    TreatmentPrivateRank0::TreatmentPrivateRank0()
    {
        // Same configuration for all terms - one private rank 0 row.
        m_configuration.push_front(RowConfiguration::Entry(0, 1, true));

        //std::cout << "Single configuration: ";
        //m_configuration.Write(std::cout);
        //std::cout << std::endl;
    }


    RowConfiguration TreatmentPrivateRank0::GetTreatment(Term /*term*/) const
    {
        return m_configuration;
    }


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0
    //
    // Terms get one or more rank 0 rows that could be private or shared,
    // depending on term frequency.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0::TreatmentPrivateSharedRank0(double density, double snr)
    {
        // Fill up vector of RowConfigurations. GetTreatment() will use the
        // IdfSum() value of the Term as an index into this vector.
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            RowConfiguration configuration;

            double frequency = Term::IdfX10ToFrequency(idf);
            if (frequency >= density)
            {
                // This term is so common that it must be assigned a private row.
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
            }
            else
            {
                int k = Term::ComputeRowCount(frequency, density, snr);
                configuration.push_front(RowConfiguration::Entry(0, k, false));
            }

            m_configurations.push_back(configuration);

            //std::cout << idf / 10.0 << ": ";
            //m_configurations.back().Write(std::cout);
            //std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0::GetTreatment(Term term) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
        return m_configurations[idf];
    }


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0And3
    //
    // Terms get one or more rank 0 and rank 3 rows that could be private or
    // shared, depending on term frequency.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0And3::TreatmentPrivateSharedRank0And3(double density, double snr)
    {
        // Fill up vector of RowConfigurations. GetTreatment() will use the
        // IdfSum() value of the Term as an index into this vector.
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            RowConfiguration configuration;

            double frequency = Term::IdfX10ToFrequency(idf);
            if (frequency > density)
            {
                // This term is so common that it must be assigned a private row.
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
            }
            else
            {
                // Determine the number of rows, k, required to reach the
                // desired signal to noise ratio, snr, given a certain bit
                // density.
                // TODO: consider checking for overflow?
                int k = Term::ComputeRowCount(frequency, density, snr);
                configuration.push_front(RowConfiguration::Entry(0, 2, false));
                if (k > 2)
                {
                    Rank rank = 3;
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank >= density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, 1, true));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, k - 2, false));
                    }
                }
            }

            m_configurations.push_back(configuration);

            //std::cout << idf / 10.0 << ": ";
            //m_configurations.back().Write(std::cout);
            //std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0And3::GetTreatment(Term term) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
        return m_configurations[idf];
    }


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0ToN
    //
    // Two rank 0 rows followed by rows of increasing rank until the bit density
    // is > some threshold. Due to limitatons in other BitFunnel code, we also
    // top out at rank 6.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0ToN::TreatmentPrivateSharedRank0ToN(double density, double snr)
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
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
            }
            else
            {
                // TODO: fix other limitations so this can be higher than 6?
                const Rank maxRank = (std::min)(Term::ComputeMaxRank(frequency, maxDensity), static_cast<Rank>(6u));

                int numRows = Term::ComputeRowCount(frequency, density, snr);
                configuration.push_front(RowConfiguration::Entry(0, 2, false));
                numRows -= 2;
                Rank rank = 1;
                while (rank < maxRank)
                {
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank >= density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1,
                                                                         true));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1,
                                                                         false));
                    }
                    ++rank;
                    --numRows;
                }

                double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                if (frequencyAtRank >= density)
                {
                    configuration.push_front(RowConfiguration::Entry(rank,
                                                                     1,
                                                                     true));
                }
                else
                {
                    if (numRows > 1)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         numRows,
                                                                         false));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1,
                                                                         false));
                    }
                }
            }

            m_configurations.push_back(configuration);

            //std::cout << idf / 10.0 << ": ";
            //m_configurations.back().Write(std::cout);
            //std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0ToN::GetTreatment(Term term) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
        return m_configurations[idf];
    }


    // Try to solve for the optimal TermTreatment. This is basically a recursive
    // brute force solution that, for any rank, either does a RankDown or
    // inserts another row at currentRank. Note that the cost function here
    // assumes that the machine has qword-sized memory accesses and that needs
    // to be updated. Furthermore, the code is basically untested and may have
    // logical errors.

    // TODO: need to ensure enough rows that we don't have too much noise. As
    // is, terms with relatively low max rank will end up with exactly 2 rank 0
    // rows or something equally silly.

    // TODO: ensure that if we have a private row, we don't also add a higher
    // rank private row? Or do we not need to do that because, if the cost
    // function is working correctly, that will only happen if it makes sense?

    // TODO: convert ranks to Rank type.
    std::pair<double, std::vector<int>> Temp(double frequency, double density, double snr, int currentRank, std::vector<int> rows, int maxRowsPerRank)
    {
        if (currentRank == 0)
        {
            if (frequency >= density)
            {
                rows[0] = 1;
            }
            else
            {
                rows[0] = 2;
            }

            // TODO: change cost calculation to properly account for cacheline size.
            double cost = 0;

            int lastRank = -1;
            double residualNoise = std::numeric_limits<double>::quiet_NaN();
            double lastFrequencyAtRank = std::numeric_limits<double>::quiet_NaN();
            double weight = 1.0; // probability that we don't have all 0s in a qword.
            for (int i = static_cast<int>(rows.size()) - 1; i >= 0; --i)
            {
                if (rows[i] != 0)
                {
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, i);
                    double noiseAtRank = (std::max)(density - frequencyAtRank, 0.0);
                    // double intersectedNoiseAtRank = pow(noiseAtRank, rows[i]);
                    double fullRowCost = 1.0 / (1 << i);

                    // Intersection with each row at rank i.
                    for (int j = 0; j < rows[i]; ++j)
                    {
                        if (j == 0)
                        {
                            if (lastRank != -1)
                            {
                                // RankDown.
                                int rankDown = lastRank - i;
                                double rankDownBits = static_cast<double>(1 << rankDown);
                                residualNoise =
                                    (lastFrequencyAtRank * (rankDownBits - 1) / rankDownBits * noiseAtRank)
                                    +
                                    (residualNoise * noiseAtRank);
                            }
                            else
                            {
                                // First intersection for this configuration.
                                residualNoise = noiseAtRank;
                            }
                        }
                        else
                        {
                            residualNoise *= noiseAtRank;
                        }
                        cost += weight * fullRowCost;
                        double densityAtRank = residualNoise + frequencyAtRank;
                        weight = 1 - pow(1 - densityAtRank, 512);

                    }

                    lastFrequencyAtRank = frequencyAtRank;
                    lastRank = i;
                }
            }

            // TODO: if we wanted to enforce a snr bound, we could set the cost
            // of anything that doesn't hit our snr to infinity.  For something
            // more nuance, we could add something to the cost function based on
            // how much we missed our target by. That seems like a better idea.
            return std::make_pair(cost, rows);
        }

        double frequencyAtRank = Term::FrequencyAtRank(frequency, currentRank);
        if (frequencyAtRank > density)
        {
            // Add private row and rankDown.
            ++rows[currentRank];
            return Temp(frequency, density, snr, currentRank - 1, rows, maxRowsPerRank);
        }
        else if (rows[currentRank] >= maxRowsPerRank)
        {
            // rankDown
            return Temp(frequency, density, snr, currentRank - 1, rows, maxRowsPerRank);
        }
        else
        {
            auto rankDown = Temp(frequency, density, snr, currentRank - 1, rows, maxRowsPerRank);
            ++rows[currentRank];
            auto newRow = Temp(frequency, density, snr, currentRank, rows, maxRowsPerRank);
            return newRow.first < rankDown.first ? newRow : rankDown;
        }
    }


    //*************************************************************************
    //
    // TreatmentExperimental
    //
    // Placeholder of experimental treatment.
    //
    //*************************************************************************
    TreatmentExperimental::TreatmentExperimental(double density, double snr)
    {
        const int c_maxRowsPerRank = 6;

        std::vector<int> rowInputs(c_maxRankValue + 1, 0);

        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            double frequency = Term::IdfX10ToFrequency(idf);
            const Rank maxRank = (std::min)(Term::ComputeMaxRank(frequency, density), static_cast<Rank>(c_maxRankValue));

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


            RowConfiguration configuration;
            for (Rank rank = 0; rank < rows.size(); ++rank)
            {
                if (rows[rank] > 0)
                {
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank > density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, 1, true));
                        // TODO: assert that our solver doesn't give us multiple
                        // private rows.
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, rows[rank], false));
                    }
                }
            }

            m_configurations.push_back(configuration);
        }
    }


    RowConfiguration TreatmentExperimental::GetTreatment(Term term) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
        return m_configurations[idf];
    }
}
