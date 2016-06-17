#pragma once

#include <iosfwd>
#include <iostream>
#include <map>
#include <mutex>

#include "BitFunnel/NonCopyable.h"
#include "CsvTsv/Csv.h"
#include "CsvTsv/Table.h"

// BitFunnelLib\src\Common\Configuration\DocumentDocumentLengthHistogram.cpp
// TODO: one unit test should be to write and read and get the same result.
// Another should be to put some known stuff in and count it correctly.

namespace BitFunnel
{
    // Note: NonCopyable is probably incompatible with map.
    class DocumentLengthHistogram : public NonCopyable
    {
    public:
        // Use CsvTsv.
        // TODO: Do we need a document scaling constructor, like in the original?
        DocumentLengthHistogram(std::istream& input);
        DocumentLengthHistogram();

        // AddDocument is thread safe with multiple writers.
        // No other method is thread safe.
        void AddDocument(size_t postingCount);
        size_t GetValue(size_t postingCount);
        // Persists the contents of the histogram to a stream
        void Write(std::ostream& output);

    private:
        std::map<size_t, size_t> m_hist;
        std::mutex m_lock;
    };
}
