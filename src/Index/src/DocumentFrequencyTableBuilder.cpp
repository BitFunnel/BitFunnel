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
            entry.first.Write(output);
            output << "," << entry.second << std::endl;
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
