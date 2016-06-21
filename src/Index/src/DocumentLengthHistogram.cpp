
#include <iostream> // TODO remove this temporary include

#include "CsvTsv/Csv.h"
#include "CsvTsv/Table.h"
#include "DocumentLengthHistogram.h"

namespace BitFunnel
{
    DocumentLengthHistogram::DocumentLengthHistogram()
    {
    }


    void DocumentLengthHistogram::AddDocument(size_t postingCount)
    {
        const std::lock_guard<std::mutex> lock(m_lock);
        if (postingCount == 0)
        {
            // TODO: Add a log assertion
        }

        ++m_hist[postingCount];
        m_totalCount += postingCount;
    }


    size_t DocumentLengthHistogram::GetValue(size_t postingCount)
    {
        const std::lock_guard<std::mutex> lock(m_lock);

        const auto kvPair = m_hist.find(postingCount);
        if (kvPair != m_hist.end())
        {
            return m_hist[postingCount];
        }
        else
        {
            return 0;
        }
    }


    void DocumentLengthHistogram::Write(std::ostream& output) const
    {
        CsvTsv::CsvTableFormatter formatter(output);
        CsvTsv::TableWriter writer(formatter);

        // TODO: Figure out how to allow `OutputColumn` to take a `size_t`, or
        // verify the cast is always valid.

        CsvTsv::OutputColumn<uint64_t> postingCount(
            "Postings",
            "Total postings in a document.");
        CsvTsv::OutputColumn<uint64_t> numDocs(
            "Count",
            "Number of documents that have a given posting count.");

        writer.DefineColumn(postingCount);
        writer.DefineColumn(numDocs);
        writer.WritePrologue();

        for (const auto & kvPairs : m_hist)
        {
            postingCount = kvPairs.first;
            numDocs = kvPairs.second;
            writer.WriteDataRow();
        }

        writer.WriteEpilogue();
    }
}
