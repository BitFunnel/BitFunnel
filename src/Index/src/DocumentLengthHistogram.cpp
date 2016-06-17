
#include <iostream>

#include "DocumentLengthHistogram.h"


namespace BitFunnel
{
    DocumentLengthHistogram::DocumentLengthHistogram()
    {
    }

    void DocumentLengthHistogram::AddDocument(size_t postingCount)
    {
        // TODO: Figure out if we need a read lock as well; it seems like this
        // operation is not atomic, and if it's not, then a reader could read
        // inconsistent data if a writer is writing at the same time.
        const std::lock_guard<std::mutex> lock(m_lock);
        if (postingCount == 0) {
            // TODO: Figure out whether we should throw if we try to add a
            // document with posting count of 0.
            throw std::runtime_error("Cannot add document with posting count "
                                     "of 0.");
        }

        ++m_hist[postingCount];
    }

    size_t DocumentLengthHistogram::GetValue(size_t postingCount)
    {
        // NOTE: An invariant of this method is that a posting count appears in
        // the map only once, i.e., it is not a multi-map.
        const auto kvPair = m_hist.find(postingCount);
        if (kvPair != m_hist.end()) {
            return m_hist[postingCount];
        } else {
            return 0;
        }
    }

    void DocumentLengthHistogram::Write(std::ostream& output)
    {
        CsvTsv::CsvTableFormatter formatter(output);
        CsvTsv::TableWriter writer(formatter);

        // TODO: Figure out how to allow `OutputColumn` to take a `size_t`, or
        // verify the cast is always valid.
        // static_assert(
        //     sizeof(unsigned int) == sizeof(size_t),
        //     "size_t and unsigned int are different sizes");
        CsvTsv::OutputColumn<unsigned int> postingCount(
            "Postings",
            "Total postings in a document.");
        CsvTsv::OutputColumn<unsigned int> numDocs(
            "Count",
            "Number of documents that have a given posting count.");

        writer.DefineColumn(postingCount);
        writer.DefineColumn(numDocs);
        writer.WritePrologue();

        for (const auto & kvPairs : m_hist) {
            postingCount = kvPairs.first;
            numDocs = kvPairs.second;
            writer.WriteDataRow();
        }

        writer.WriteEpilogue();
    }
}
