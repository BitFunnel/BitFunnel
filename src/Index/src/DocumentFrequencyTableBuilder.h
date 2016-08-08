#pragma once

#include <iosfwd>           // std::ostream parameter.
#include <unordered_map>    // std::unordered_map member.
#include <vector>           // std::vector member.

#include "BitFunnel/Term.h" // Term and Term::Hasher template parameters.


namespace BitFunnel
{
    //*************************************************************************
    //
    // DocumentFrequencyTableBuilder
    //
    // Generates statistics on Term extracted from a corpus of documents.
    //
    // The first statistic is a Document Frequency Table which tracks the
    // number of documents in which each term appears. The Document Frequency
    // Table is represented internally as a map from Term to count.
    //
    // The second statistic the Cumulative Posting Count table which tracks
    // the number of unique terms in the Document Frequency Table as a function
    // of the number of documents processed so far.
    //
    // Information about the corpus is supplied to the DocumentFrequencyTableBuilder
    // through a sequence of calls to OnDocumentEnter() and OnTerm().
    //
    // OnDocumentEnter() should be called once for each document. Then OnTerm()
    // is called once for each unique term in the document. OnDocumentEnter()
    // should not be called again until all terms in the current document have
    // been recorded via calls to OnTerm().
    //
    //*************************************************************************
    class DocumentFrequencyTableBuilder
    {
    public:
        void OnDocumentEnter();
        void OnTerm(Term t);

        // Writes the Document Frequency Table to a stream. The file format is
        // a sequence of entries, one per line. Each entry consists of the
        // following comma-separated fields:
        //    term hash (16 digit hexidecimal)
        //    gram size (e.g. 1 for unigram, 2 for bigram phrase, etc.)
        //    stream id (e.g. 0 for body, 1 for title, etc.)
        //    frequency of term in corpus (double precision floating point)
        // Entries are ordered by decreasing frequency.
        // The list is truncated at the truncateBelowFrequency.
        void WriteFrequencies(std::ostream& output,
                              double truncateBelowFrequency) const;

        // Writes the Cumulative Posting Count Table to a stream.
        // The file format is a series of entries, one per line. Each entry
        // consists of the following comm-separated fields:
        //    document count (integer)
        //    unique term count (integer)
        // Entries are ordered by increasing document count.
        void WriteCumulativePostingCounts(std::ostream& output) const;

    private:
        std::vector<size_t> m_cumulativePostingCounts;
        std::unordered_map<Term, size_t, Term::Hasher> m_termCounts;
    };
}
