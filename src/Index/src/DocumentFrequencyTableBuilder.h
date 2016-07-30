#pragma once

#include <iosfwd>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BitFunnel/Term.h"

namespace BitFunnel
{
    class DocumentFrequencyTableBuilder
    {
    public:
        void OnDocumentEnter();
        void OnTerm(Term t);

        void WriteFrequencies(std::ostream& output, double truncateBelowFrequency) const;
        void WriteCumulativeCounts(std::ostream& output) const;

    private:
        std::vector<size_t> m_cumulativeTermCounts;
        // std::map<Term, size_t, Term::Hasher> m_termCounts;
        std::unordered_map<Term, size_t, Term::Hasher> m_termCounts;
    };
}
