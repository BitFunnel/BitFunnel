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
#include <ostream>
#include <stddef.h>
#include <utility>
#include <vector>

#include "BitFunnel/Index/ITermTreatment.h"
#include "BitFunnel/Term.h"
#include "LoggerInterfaces/Check.h"
#include "OptimalTermTreatments.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    // Computes the TermTreatmentMetrics for a single configuration, given
    // a particular density and signal. The configuration is a 6-digit number
    // where the 10^r digit has the number of rows at rank r. For example,
    // the configuration 000321 corresponds to 3 rank 2 rows, 2 rank 1 rows,
    // and a single rank 0 row.
    std::pair<bool, TermTreatmentMetrics> Analyze(size_t configuration,
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
        for (int rank = static_cast<int>(rowsAtRank.size()) - 1; rank >= 0; --rank)
        {
            unsigned fanout = 1u << rank;
            double signalAtRank = Term::FrequencyAtRank(signal, rank);
            double correlatedNoise = signalAtRank - signal;
            uncorrelatedNoise += (previousCorrelatedNoise - correlatedNoise);

            // TODO: Can't comment this out since it protects against negative noise.
            //if (signalAtRank > density)
            //{
            //    return std::make_pair(false, TermTreatmentMetrics());
            //}

            double noise = density - signalAtRank;
            if (noise < 0.0)
            {
                // This must be a private row.
                noise = 0.0;
            }

            for (size_t i = 0; i < rowsAtRank[rank]; ++i)
            {
                expectedQuadwordReads += pQuadwordRead / fanout;

                uncorrelatedNoise *= noise;

                double totalNoise = uncorrelatedNoise + correlatedNoise;
                double bitIsZero = 1.0 - signal - totalNoise;
                //double bitIsZero = (1.0 - totalNoise) * (1.0 - signal);   // Incorrect formulation.
                pQuadwordRead = 1.0 - pow(bitIsZero, 64);

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

        return std::make_pair(
            true,
            TermTreatmentMetrics(signal / (uncorrelatedNoise + previousCorrelatedNoise),
                                 expectedQuadwordReads,
                                 bitsPerDocument));
    }


    size_t FindBestTreatment(double density, double signal, double snr)
    {
        // Variables used to track the best row configuration and its
        // TermTreatmentMetrics.
        size_t bestConfiguration = 0;
        TermTreatmentMetrics bestResult(0.0,
                                        std::numeric_limits<double>::infinity(),
                                        std::numeric_limits<double>::infinity());

        // Enumerate all row configurations.
        for (size_t configuration = 1; configuration <= 99999; ++configuration)
        {
            auto result = Analyze(configuration, density, signal, false);
            if (result.first)
            {
                // result.first == true implies that this configuration is valid.
                TermTreatmentMetrics const & metrics = result.second;

                // We only consider configurations that result in a snr above the threshold.
                if (metrics.GetSNR() >= snr)
                {
                    // configuration is a candidate.
                    if (metrics.GetDQ() > bestResult.GetDQ())
                    {
                        // configuration is the best candidate seen so far.
                        bestResult = metrics;
                        bestConfiguration = configuration;
                    }
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


    void OptimalTermTreatments()
    {
        const double density = 0.1;
        const double snr = 10.0;

        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            double signal = Term::IdfX10ToFrequency(idf);
            //pow(10.0, -0.1 * idf);

            // Algorithm only valid for terms with signal <= density.
            // TODO: Understand why signal >= density seems to lead to term
            // treatments with higher rank rows.
            if (signal < density)
            {
                std::cout
                    << std::setprecision(2) << "idf " << 0.1 * idf
                    << ": " << std::setprecision(6);
                FindBestTreatment(density, signal, snr);
            }
            else
            {
                std::cout
                    << "idf " << std::setprecision(2) << 0.1 * idf
                    << ": " << std::setprecision(6) << signal
                    << " ==> private row" << std::endl;
            }
        }
    }


    RowConfiguration RowConfigurationFromSizeT(size_t configuration)
    {
        RowConfiguration rc;

        Rank rank = 0;
        while (configuration != 0)
        {
            size_t count = configuration % 10;
            if (count != 0)
            {
                rc.push_front(RowConfiguration::Entry(rank, count, false));
            }
            configuration /= 10;
            rank++;
        }

        return rc;
    }


    size_t SizeTFromRowConfiguration(RowConfiguration rc)
    {
        size_t result = 0;

        for (auto entry : rc)
        {
            size_t digit = 1;
            for (size_t i = 0; i < entry.GetRank(); ++i)
            {
                digit *= 10;
            }
            result += entry.GetRowCount() * digit;
        }

        return result;
    }


    void AnalyzeTermTreatment(ITermTreatment const & treatment,
                              double density)
    {
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            //if (idf == 10)
            {
                Term term(0ull, 0u, idf);
                auto rc = treatment.GetTreatment(term);
                auto configuration = SizeTFromRowConfiguration(rc);

                double signal = Term::IdfX10ToFrequency(idf);

                auto result = Analyze(configuration, density, signal, false);

                std::cout
                    << std::setprecision(2) << "idf " << 0.1 * idf
                    << ": " << std::setprecision(6)
                    << signal
                    << " ==> "
                    << std::setw(6) << std::setfill('0') << configuration
                    << ": ";

                result.second.Print(std::cout);
            }
        }
    }


    //*************************************************************************
    //
    // TreatmentOptimal
    //
    //*************************************************************************
    TreatmentOptimal::TreatmentOptimal(double density, double snr)
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
                m_configurations.push_back(RowConfigurationFromSizeT(config));
            }
            else
            {
                std::cout
                    << "idf " << std::setprecision(2) << 0.1 * idf
                    << ": " << std::setprecision(6) << signal
                    << " ==> private row" << std::endl;
                RowConfiguration configuration;
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
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
}
