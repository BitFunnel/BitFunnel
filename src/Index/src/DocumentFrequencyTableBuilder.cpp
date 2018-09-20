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
#include <iostream>
#include <vector>

#include "DocumentFrequencyTable.h"
#include "DocumentFrequencyTableBuilder.h"


namespace BitFunnel
{
    void DocumentFrequencyTableBuilder::OnDocumentEnter()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_cumulativeTermCounts.push_back(m_termCounts.size());
    }


    void DocumentFrequencyTableBuilder::OnTerm(Term t)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        ++m_termCounts[t];
    }


    // Write out sorted truncated list, sorted by count (TODO: frequency).
    void DocumentFrequencyTableBuilder::WriteFrequencies(std::ostream& output,
                                                         double truncateBelowFrequency,
                                                         ITermToText const * termToText) const
    {
        DocumentFrequencyTable table;

        // For each term count record, compute the document frequency then
        // add to entries if frequency is above threshold.
        for (auto const & entry : m_termCounts)
        {
            double frequency = static_cast<double>(entry.second) / m_cumulativeTermCounts.size();
            if (frequency >= truncateBelowFrequency)
            {
                table.AddEntry(DocumentFrequencyTable::Entry(entry.first, frequency));
            }
        }

        table.Write(output, termToText);

        std::cout << "Raw DocumentFrequencyTable count: "
                  << m_termCounts.size()
                  << std::endl
                  << "Saved DocumentFrequencyTable count: "
                  << table.size()
                  << std::endl;
    }


    void DocumentFrequencyTableBuilder::WriteCumulativeTermCounts(std::ostream& output) const
    {
        for (size_t i = 0; i < m_cumulativeTermCounts.size(); ++i)
        {
            output << i << "," << m_cumulativeTermCounts[i] << std::endl;
        }
    }
}
