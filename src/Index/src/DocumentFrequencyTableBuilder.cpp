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
#include <utility>

#include "DocumentFrequencyTableBuilder.h"


namespace BitFunnel
{
    void DocumentFrequencyTableBuilder::OnDocumentEnter()
    {
        m_cumulativePostingCounts.push_back(m_termCounts.size());
    }


    void DocumentFrequencyTableBuilder::OnTerm(Term t)
    {
        ++m_termCounts[t];
    }


    // Write out sorted truncated list, sorted by count (TODO: frequency).
    void DocumentFrequencyTableBuilder::WriteFrequencies(std::ostream& output, double truncateBelowFrequency) const
    {
        typedef std::pair<Term, double> Entry;

        struct
        {
            bool operator() (Entry a, Entry b)
            {
                // Sorts by decreasing frequency.
                return a.second > b.second;
            }
        } compare;

        // Storage for sorted entries.
        std::vector<Entry> entries;

        // For each term count record, compute the document frequency then
        // add to entries if frequency is above threshold.
        for (auto const & entry : m_termCounts)
        {
            double frequency = static_cast<double>(entry.second) / m_cumulativePostingCounts.size();
            if (frequency >= truncateBelowFrequency)
            {
                entries.push_back(std::make_pair(entry.first, frequency));
            }
        }

        // Sort document frequency records by decreasing frequency.
        std::sort(entries.begin(), entries.end(), compare);

        // Write sorted list to stream.
        for (auto const & entry : entries)
        {
            output << std::hex << entry.first.GetRawHash()
                   << ","
                   << std::dec << static_cast<unsigned>(entry.first.GetGramSize())
                   << ","
                   << static_cast<unsigned>(entry.first.GetStream())
                   << ","
                   << entry.second
                   << std::endl;
        }
    }


    void DocumentFrequencyTableBuilder::WriteCumulativePostingCounts(std::ostream& output) const
    {
        for (size_t i = 0; i < m_cumulativePostingCounts.size(); ++i)
        {
            output << i << "," << m_cumulativePostingCounts[i] << std::endl;
        }
    }
}
