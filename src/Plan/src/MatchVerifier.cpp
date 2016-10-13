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
#include <ostream>

#include "BitFunnel/Plan/Factories.h"
#include "MatchVerifier.h"

namespace BitFunnel
{
    std::unique_ptr<IMatchVerifier> Factories::CreateMatchVerifier()
    {
        return std::unique_ptr<IMatchVerifier>(new MatchVerifier);
    }


    void MatchVerifier::AddExpected(DocId id)
    {
        m_expected.push_back(id);
    }


    void MatchVerifier::AddObserved(DocId id)
    {
        m_observed.push_back(id);
    }


    void MatchVerifier::Verify()
    {
        // Sort from smallest to largest.
        std::sort(m_expected.begin(), m_expected.end());
        std::sort(m_observed.begin(), m_observed.end());

        // Indices into expected and observed.
        size_t e = 0;
        size_t o = 0;
        while (e < m_expected.size() && o < m_observed.size())
        {
            DocId expected = m_expected[e];
            DocId observed = m_observed[o];
            if (expected < observed)
            {
                m_falseNegatives.push_back(expected);
                ++e;
            }
            else if (expected > observed)
            {
                m_falsePositives.push_back(observed);
                ++o;
            }
            else // observed == expected
            {
                m_truePositives.push_back(expected);
                ++o;
                ++e;
            }
        }

        while (o < m_observed.size())
        {
            m_falsePositives.push_back(m_observed[o]);
            ++o;
        }

        while (e < m_expected.size())
        {
            m_falseNegatives.push_back(m_expected[e]);
            ++e;
        }
    }


    void PrintDocIdVector(std::ostream & out,
                          std::vector<DocId> const & documents)
    {
        for (size_t i = 0; i < documents.size(); ++i)
        {
            if (i > 0)
            {
                out << ", ";
            }
            out << documents[i];
        }
        out << std::endl;
    }


    void MatchVerifier::Print(std::ostream & out) const
    {
        out << "False Positives: ";
        PrintDocIdVector(out, m_falsePositives);
        out << std::endl;

        out << "False Negatives: ";
        PrintDocIdVector(out, m_falseNegatives);
        out << std::endl;

        out << "True Positives: ";
        PrintDocIdVector(out, m_truePositives);
        out << std::endl;

        out << "False Positives: "
            << m_falsePositives.size();

        size_t totalPositives =
            m_falsePositives.size() + m_truePositives.size();
        if (totalPositives > 0)
        {
            out
                << " (rate = "
                << static_cast<double>(
                    m_falsePositives.size()) /
                totalPositives
                << ") ";
        }

        out << std::endl
            << "False Negatives: "
            << m_falseNegatives.size()
            << std::endl
            << "True Positives: "
            << m_truePositives.size()
            << std::endl;
    }


    void MatchVerifier::Reset()
    {
        m_expected.clear();
        m_observed.clear();
        m_truePositives.clear();
        m_falsePositives.clear();
        m_falseNegatives.clear();
    }
}
