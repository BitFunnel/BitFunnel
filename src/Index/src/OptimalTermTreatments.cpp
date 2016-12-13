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

#include <iostream>
#include <ostream>
#include <stddef.h>
#include <vector>
#include <cmath>

#include "BitFunnel/Term.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermTreatmentMetrics
    //
    // Stores the signal-to-noise ratio (SNR), expected number of quadwords
    // accessed per matcher iteration, and number of bits per document used
    // for terms with this treatment.
    //
    // Note that a TermTreatmentMetric constructed in the context of a
    // particular combination of row configuration, density and signal.
    //
    //*************************************************************************
    class TermTreatmentMetrics
    {
    public:
        TermTreatmentMetrics()
          : m_snr(0.0),
            m_quadwords(0.0),
            m_bits(0.0)
        {
        }


        TermTreatmentMetrics(double snr, double quadwords, double bits)
            : m_snr(snr),
            m_quadwords(quadwords),
            m_bits(bits)
        {
        }


        double GetSNR() const
        {
            return m_snr;
        }


        double GetQuadwords() const
        {
            return m_quadwords;
        }


        double GetBits() const
        {
            return m_bits;
        }


        double GetDQ() const
        {
            return 1.0 / m_quadwords / m_bits;
        }


        static TermTreatmentMetrics NoResult()
        {
            return TermTreatmentMetrics();
        }


        bool IsValid() const
        {
            return !(m_snr == 0 && m_quadwords == 0 && m_bits == 0);
        }


        void Print(std::ostream& out)
        {
            out << "snr: " << m_snr
                << ", qw: " << m_quadwords
                << ", bits: " << m_bits
                << ", dq: " << GetDQ()
                << std::endl;
        }

    private:
        double m_snr;
        double m_quadwords;
        double m_bits;
    };


    // Computes the TermTreatmentMetrics for a single configuration, given
    // a particular density and signal. The configuration is a 6-digit number
    // where the 10^r digit has the number of rows at rank r. For example,
    // the configuration 000321 corresponds to 3 rank 2 rows, 2 rank 1 rows,
    // and a single rank 0 row.
    TermTreatmentMetrics Analyze(size_t configuration,
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
            double signalAtRank = 1.0 - pow(1.0 - signal, fanout);
            double correlatedNoise = signalAtRank - signal;
            uncorrelatedNoise += (previousCorrelatedNoise - correlatedNoise);

            if (signalAtRank > density)
            {
                TermTreatmentMetrics::NoResult();
            }

            double noise = density - signalAtRank;

            for (size_t i = 0; i < rowsAtRank[rank]; ++i)
            {
                expectedQuadwordReads += pQuadwordRead / fanout;

                uncorrelatedNoise *= noise;

                double totalNoise = uncorrelatedNoise + correlatedNoise;
                double bitIsZero = (1.0 - totalNoise) * (1.0 - signal);
                pQuadwordRead = 1.0 - pow(bitIsZero, 64);

                bitsPerDocument += signalAtRank / density;

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

            // We only consider configurations that result in a snr above the threshold.
            if (result.GetSNR() >= snr)
            {
                // configuration is a candidate.
                if (result.GetDQ() > bestResult.GetDQ())
                {
                    // configuration is the best candidate seen so far.
                    bestResult = result;
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
            << bestConfiguration
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
            double signal = pow(10.0, -0.1 * idf);

            // Algorithm only valid for terms with signal <= density.
            // TODO: Understand why signal >= density seems to lead to term
            // treatments with higher rank rows.
            if (signal <= density)
            {
                std::cout << "idf " << 0.1 * idf << ": ";
                FindBestTreatment(density, signal, snr);
            }
        }
    }
}
