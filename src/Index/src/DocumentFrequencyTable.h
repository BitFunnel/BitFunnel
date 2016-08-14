#pragma once

#include <iosfwd>                                       // std::istream member.
#include <utility>                                      // std::pair return value.
#include <vector>                                       // std::vector member.

#include "BitFunnel/Index/IDocumentFrequencyTable.h"    // Base class.
#include "BitFunnel/Term.h"                             // Term template parameter.


namespace BitFunnel
{
    class DocumentFrequencyTable : public IDocumentFrequencyTable
    {
    public:
        // Constructs an empty DocumentFrequencyTable.
        DocumentFrequencyTable();

        // Constructs a DocumentFrequencyTable from data previously persisted
        // to a stream by DocumentFrequencyTableBuilder::WriteFrequencies().
        // The file format is a sequence of entries, one per line. Each entry
        // consists of the following comma-separated fields:
        //    term hash (16 digit hexidecimal)
        //    gram size (e.g. 1 for unigram, 2 for bigram phrase, etc.)
        //    stream id (e.g. 0 for body, 1 for title, etc.)
        //    frequency of term in corpus (double precision floating point)
        // Entries must be ordered by non-increasing frequency.
        DocumentFrequencyTable(std::istream& input);

        // Sorts the entries by descending frequency then writes to a stream.
        // If the optional termToText parameter is not nullptr, the file will
        // contain a "text" column with the text for each term, if available
        // via the ITermToText. Note: method is not const because it sorts
        // the entries.
        virtual void Write(std::ostream & output,
                           TermToText const * termToText) override;

        // Adds an Entry to the table. Note that this method does not guard
        // against duplicate Term::Hash values and it does not enforce any
        // ordering on the frequencies. Entries are sorted on write.
        virtual void AddEntry(Entry const & entry) override;

        // Returns the entry corresponding a specific index.
        virtual Entry const & operator[](size_t index) const override;

        virtual std::vector<Entry>::const_iterator begin() const override;
        virtual std::vector<Entry>::const_iterator end() const override;

        virtual size_t size() const override;

    private:
        std::vector<Entry> m_entries;
    };
}
