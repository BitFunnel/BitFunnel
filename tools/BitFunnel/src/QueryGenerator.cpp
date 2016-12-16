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

#include <random>
#include <string>

#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/Index/ITermToText.h"
#include "QueryGenerator.h"


namespace BitFunnel
{
    QueryGenerator::QueryGenerator(
        IDocumentFrequencyTable const & dft,
        ITermToText const & terms)
      : m_dft(dft),
        m_terms(terms),
        m_distribution(0.0, 1.0)
    {
        // Generate and store a large number of random values while tracking
        // the maximum value.
        m_maxValue = 0.0;
        m_values.reserve(c_valueCount);
        for (size_t i = 0; i < c_valueCount; ++i)
        {
            double value = m_distribution(m_generator);
            if (value > m_maxValue)
            {
                m_maxValue = value;
            }
            m_values.push_back(value);
        }

        m_nextValue = 0;
    }


    std::string QueryGenerator::CreateOneQuery(size_t termCount)
    {
        std::string query;
        query.reserve(200);

        for (size_t i = 0; i < termCount; ++i)
        {
            if (i != 0)
            {
                query.push_back(' ');
            }
            AppendOneTerm(query);
        }

        return query;
    }


    void QueryGenerator::AppendOneTerm(std::string& query)
    {
        // Grab the next random number
        double value = m_values[m_nextValue % m_values.size()];
        m_nextValue++;

        // Convert random value to an index into the document frequency table.
        value /= m_maxValue;
        value *= m_dft.size();

        size_t index = static_cast<size_t>(value);

        auto entry = m_dft[index];

        query.append(m_terms.Lookup(entry.GetTerm().GetRawHash()));
    }
}
