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

#pragma once

#include <cmath>  // Used for pow in TermTreatmentMetrics.
#include <utility>  // std::pair.

namespace BitFunnel
{
    class ITermTreatment;

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
            return 1.0 / (m_quadwords * m_bits);
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

    void OptimalTermTreatments();
    void AnalyzeTermTreatment(ITermTreatment const & treatment,
                              double density);

    // TODO: maybe remove this. If not, put in right place.
    std::pair<bool, TermTreatmentMetrics> Analyze(size_t configuration,
                                                  double density,
                                                  double signal,
                                                  bool verbose);
}
