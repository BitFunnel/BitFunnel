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
#include <cmath>
#include <iomanip>
#include <iostream>

#include "TreatmentOptimal.h"


namespace BitFunnel
{
    class Analyzer
    {
    public:
        static size_t FindOptimalConfiguration(double density,
                                               double signal,
                                               double snr);

    private:
        Analyzer(double density, double signal, double snr);

        size_t Go();

        void Recursion(size_t rank,
                       bool firstRow,
                       size_t config,
                       double uncorrelatedNoise,
                       double previousCorrelatedNoise,
                       double expectedQuadwordsReads,
                       double pQuadwordRead,
                       double bitsPerDocument);

        double m_density;
        double m_signal;
        double m_snr;

        double m_bestDQ;
        size_t m_bestConfiguration;

        const size_t m_maxRowsPerRank = 9;
    };


    Analyzer::Analyzer(double density, double signal, double snr)
      : m_density(density),
        m_signal(signal),
        m_snr(snr)
    {
    }


    size_t Analyzer::FindOptimalConfiguration(double density, double signal, double snr)
    {
        Analyzer analyzer(density, signal, snr);
        return analyzer.Go();
    }


    size_t Analyzer::Go()
    {
        m_bestDQ = -1;
        m_bestConfiguration = 0ull;

        size_t maxRank = 5;
        bool firstRow = true;
        size_t config = 0;
        double uncorrelatedNoise = 0.0;
        double previousCorrelatedNoise = 1.0;
        double expectedQuadwordReads = 0.0;
        double pQuadwordRead = 1.0;
        double bitsPerDocument = 0.0;

        for (size_t rank = 0; rank <= maxRank; ++rank)
        {
            Recursion(rank,
                      firstRow,
                      config,
                      uncorrelatedNoise,
                      previousCorrelatedNoise,
                      expectedQuadwordReads,
                      pQuadwordRead,
                      bitsPerDocument);
        }

        return m_bestConfiguration;
    }


    void Analyzer::Recursion(size_t rank,
                             bool firstRow,
                             size_t configuration,
                             double uncorrelatedNoise,
                             double previousCorrelatedNoise,
                             double expectedQuadwordReads,
                             double pQuadwordRead,
                             double bitsPerDocument)
    {
        size_t fanout = 1ull << rank;
        double signalAtRank = Term::FrequencyAtRank(m_signal, rank);
        double correlatedNoise = signalAtRank - m_signal;
        uncorrelatedNoise += (previousCorrelatedNoise - correlatedNoise);

        double noise = m_density - signalAtRank;
        if (noise < 0.0)
        {
            // This must be a private row.
            noise = 0.0;
        }

        for (size_t rowCount = 0;;)
        {
            //std::cout << "Rank: " << rank
            //    << " Config: " << configuration
            //    << " PreviousCorrelatedNoise: " << previousCorrelatedNoise
            //    << " snr = " << m_signal / (uncorrelatedNoise + correlatedNoise)
            //    << " dq = " << 1.0 / (expectedQuadwordReads * bitsPerDocument)
            //    << std::endl;

            if (!firstRow)
            {
                if (rank > 0)
                {
                    Recursion(rank - 1,
                              firstRow,
                              configuration * 10,
                              uncorrelatedNoise,
                              correlatedNoise,
                              expectedQuadwordReads,
                              pQuadwordRead,
                              bitsPerDocument);
                }
            }

            if (rowCount >= m_maxRowsPerRank)
            {
                break;
            }

            // Add another row.
            ++rowCount;
            ++configuration;

            expectedQuadwordReads += pQuadwordRead / fanout;

            if (firstRow)
            {
                uncorrelatedNoise = noise;
                firstRow = false;
            }
            else
            {
                uncorrelatedNoise *= noise;
            }

            double totalNoise = uncorrelatedNoise + correlatedNoise;
            double bitIsZero = 1.0 - m_signal - totalNoise;
            pQuadwordRead = 1.0 - pow(bitIsZero, 64);

            if (signalAtRank >= m_density)
            {
                bitsPerDocument += 1.0 / fanout;
            }
            else
            {
                bitsPerDocument += signalAtRank / m_density;
            }

            if (rank == 0)
            {
                // Base case.
                double snr = m_signal / totalNoise;
                if (snr >= m_snr)
                {
                    double dq = 1.0 / (expectedQuadwordReads * bitsPerDocument);
                    if (dq > m_bestDQ)
                    {
                        m_bestDQ = dq;
                        m_bestConfiguration = configuration;
                        // Not sure whether one can prune the search here.
                        // Need to prove that it is ok.
                        // Not sure if it is any faster.
                        // return;
                    }
                }
            }
        }
    }


    //*************************************************************************
    //
    // TreatmentOptimal
    //
    //*************************************************************************
    TreatmentOptimal::TreatmentOptimal(double density,
                                       double snr)
    {
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            double signal = Term::IdfX10ToFrequency(idf);

            // Algorithm only valid for terms with signal <= density.
            // TODO: Understand why signal >= density seems to lead to term
            // treatments with higher rank rows.
            if (signal < density)
            {
                std::cout
                    << std::setprecision(2) << "idf " << 0.1 * idf
                    << ": " << std::setprecision(6);
                auto config = FindBestTreatment(density, signal, snr);
                m_configurations.push_back(RowConfiguration(config));
            }
            else
            {
                std::cout
                    << "idf " << std::setprecision(2) << 0.1 * idf
                    << ": " << std::setprecision(6) << signal
                    << " ==> private row" << std::endl;
                RowConfiguration configuration;
                configuration.push_front(RowConfiguration::Entry(0, 1));
                m_configurations.push_back(configuration);
            }
        }
    }


    RowConfiguration TreatmentOptimal::GetTreatment(Term term) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
        return m_configurations[idf];
    }


    size_t TreatmentOptimal::FindBestTreatment(double density,
                                               double signal,
                                               double snr)
    {
        size_t configuration = Analyzer::FindOptimalConfiguration(density, signal, snr);
        std::cout << "Best configuration is " << configuration << std::endl;
        return configuration;
    }
}
