#include <algorithm>
#include <iostream>
#include <utility>

#include "DocumentFrequencyTableBuilder.h"


namespace BitFunnel
{
    DocumentFrequencyTableBuilder::DocumentFrequencyTableBuilder()
    {
    }


    void DocumentFrequencyTableBuilder::OnDocumentEnter()
    {
        m_cumulativeTermCounts.push_back(m_termCounts.size());
    }


    void DocumentFrequencyTableBuilder::OnTerm(Term t)
    {
        ++m_termCounts[t];
    }


    // Write out sorted truncated list, sorted by count (TODO: frequency).
    void DocumentFrequencyTableBuilder::WriteFrequencies(std::ostream& output, double truncateBelowFrequency) const
    {
        typedef std::pair<Term, size_t> Entry;
        std::vector<Entry> entries;
        struct
        {
            bool operator() (Entry a, Entry b)
            {
                return a.second < b.second;
            }
        } compare;

        for (auto const & entry : m_termCounts)
        {
            // TODO: this makes no sense -- entry is a count.
            if (entry.second >= truncateBelowFrequency)
            {
                entries.push_back(entry);
            }
        }

        std::sort(entries.begin(), entries.end(), compare);

        for (auto const & entry : entries)
        {
            entry.first.Write(output);
            output << "," << entry.second << std::endl;
        }
    }


    void DocumentFrequencyTableBuilder::WriteCumulativeCounts(std::ostream& /*output*/) const
    {
    }
}
