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
#include <vector>

#include "TreatmentOptimalOld.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TreatmentOptimalOld
    //
    //*************************************************************************
    TreatmentOptimalOld::TreatmentOptimalOld(double density, double snr)
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


    RowConfiguration TreatmentOptimalOld::GetTreatment(Term::IdfX10 idf) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        idf = std::min(idf, local);
        return m_configurations[idf];
    }


    // Computes the TermTreatmentMetrics for a single configuration, given
    // a particular density and signal. The configuration is a 6-digit number
    // where the 10^r digit has the number of rows at rank r. For example,
    // the configuration 000321 corresponds to 3 rank 2 rows, 2 rank 1 rows,
    // and a single rank 0 row.
    // TODO: This doesn't need to return a pair.
    TermTreatmentMetrics
        TreatmentOptimalOld::Analyze(size_t configuration,
                                     double density,
                                     double signal,
                                     bool verbose)
    {
        // Unpack configuration.
        std::vector<size_t> rowsAtRank;
        while (configuration != 0)
        {
            rowsAtRank.push_back(configuration % 10);
            configuration /= 10;
        }

        if (verbose)
        {
            std::cout << "Rows: ";
            for (size_t i = 0; i < rowsAtRank.size(); ++i)
            {
                if (i > 0)
                {
                    std::cout << ", ";
                }
                std::cout << rowsAtRank[i];
            }
            std::cout << std::endl << std::endl;
        }

        double uncorrelatedNoise = 0.0;
        double previousCorrelatedNoise = 1.0;
        double expectedQuadwordReads = 0.0;
        double pQuadwordRead = 1.0;
        double bitsPerDocument = 0.0;

        size_t row = 0;
        // NOTE: For loop uses signed type because it counts down to 0.
        bool firstIntersection = true;
        for (int rank = static_cast<int>(rowsAtRank.size()) - 1; rank >= 0; --rank)
        {
            unsigned fanout = 1u << rank;
            double signalAtRank = Term::FrequencyAtRank(signal, rank);
            double correlatedNoise = signalAtRank - signal;
            uncorrelatedNoise += (previousCorrelatedNoise - correlatedNoise);

            if (verbose)
            {
                std::cout << "-----rank:uncorrelatedNoise:correlatedNoise "
                          << rank << ":"
                          << uncorrelatedNoise << ":"
                          << correlatedNoise << std::endl;
            }

            double noise = density - signalAtRank;
            if (noise < 0.0)
            {
                // This must be a private row.
                noise = 0.0;
            }

            for (size_t i = 0; i < rowsAtRank[rank]; ++i)
            {
                expectedQuadwordReads += pQuadwordRead / fanout;

                if (firstIntersection)
                {
                    uncorrelatedNoise = noise;
                    firstIntersection = false;
                }
                else
                {
                    uncorrelatedNoise *= noise;
                }

                double totalNoise = uncorrelatedNoise + correlatedNoise;
                double bitIsZero = 1.0 - signal - totalNoise;
                pQuadwordRead = 1.0 - pow(bitIsZero, 64);

                if (verbose)
                {
                        std::cout << "totalNoise:totalBits:PNonZero "
                                  << totalNoise << ":"
                                  << 1.0 - bitIsZero << ":"
                                  << pQuadwordRead << std::endl;
                }

                // TODO: bitsPerDocument below is wrong for private rows.
                // In private rows, we need one bit per document.
                if (signalAtRank >= density)
                {
                    bitsPerDocument += 1.0 / fanout;
                }
                else
                {
                    bitsPerDocument += signalAtRank / density;
                }

                if (verbose)
                {
                    std::cout
                        << "row(" << row << ", " << rank << "): "
                        << uncorrelatedNoise
                        << ", " << correlatedNoise
                        << ", " << signal / (totalNoise)
                        << ", " << expectedQuadwordReads
                        << ", " << bitsPerDocument
                        << ", " << 1.0 / expectedQuadwordReads / bitsPerDocument
                        << std::endl;
                }
                ++row;
            }

            previousCorrelatedNoise = correlatedNoise;
        }

        if (verbose)
        {
            std::cout << std::endl;
        }

        return TermTreatmentMetrics(signal / (uncorrelatedNoise + previousCorrelatedNoise),
                                    expectedQuadwordReads,
                                    bitsPerDocument);
    }


    size_t TreatmentOptimalOld::FindBestTreatment(double density,
                                                  double signal,
                                                  double snr)
    {
        // Variables used to track the best row configuration and its
        // TermTreatmentMetrics.
        size_t bestConfiguration = 0;
        TermTreatmentMetrics bestResult(0.0,
                                        std::numeric_limits<double>::infinity(),
                                        std::numeric_limits<double>::infinity());

        // Enumerate all row configurations.
        // TODO: Compute 999999 algorithmically. Should go up to max rank in use.
        for (size_t configuration = 1; configuration <= 999999; ++configuration)
        {
            auto metrics = Analyze(configuration, density, signal, false);

            // We only consider configurations that result in a snr above the threshold.
            if (metrics.GetSNR() >= snr)
            {
                if (metrics.GetDQ() > bestResult.GetDQ())
                {
                    bestResult = metrics;
                    bestConfiguration = configuration;
                }
            }
        }

        if (bestConfiguration == 0)
        {
            // TODO: Throw some sort of error here? Return private row?
            std::cout << "No configuration meets SNR criteria." << std::endl;
        }

        std::cout
            << signal
            << " ==> "
            << std::setw(6) << std::setfill('0') << bestConfiguration
            << ": ";
        bestResult.Print(std::cout);

        return bestConfiguration;
    }
}
