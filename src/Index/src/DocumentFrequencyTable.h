#pragma once

#include <iosfwd>
#include <utility>
#include <vector>

#include "BitFunnel/Term.h"

namespace BitFunnel
{
    class DocumentFrequencyTable
    {
    public:
        // TODO: unifiy with other use of this?
        typedef std::pair<Term, double> Entry;

        DocumentFrequencyTable(std::istream& input);

        Entry const & operator[](size_t index) const;

        std::vector<Entry>::iterator begin() const;
        std::vector<Entry>::iterator end() const;

        size_t size() const;
    private:
        std::vector<Entry> m_entries;
    };
}
