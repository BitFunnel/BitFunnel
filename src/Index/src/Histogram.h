#pragma once

#include <iosfwd>
#include <map>
#include <mutex>
#include "BitFunnel/NonCopyable.h"

// BitFunnelLib\src\Common\Configuration\DocumentHistogram.cpp
// TODO: one unit test should be to write and read and get the same result.
// Another should be to put some known stuff in and count it correctly.

namespace BitFunnel
{
    // Note: NonCopyable is probably incompatible with map.
    class Histogram : public NonCopyable, public std::map<size_t, size_t>
    {
    public:
        // Use CsvTsv.
        // TODO: Do we need a document scaling constructor, like in the original?
        Histogram(std::istream& input);
        Histogram();

        // AddDocument is thread safe with multiple writers.
        // No other method is thread safe.
        void AddDocument(size_t postingCount);
        void Write(std::ostream& output) const;

    private:
        std::mutex m_lock;
    };
}
