#pragma once

#include <iosfwd>
#include <unordered_map>
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
        void WriteCumulativePostingCounts(std::ostream& output) const;

    private:
        std::vector<size_t> m_cumulativePostingCounts;
        // std::map<Term, size_t, Term::Hasher> m_termCounts;
        std::unordered_map<Term, size_t, Term::Hasher> m_termCounts;
    };
}
