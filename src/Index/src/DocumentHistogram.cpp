// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "CsvTsv/Csv.h"
#include "DocumentHistogram.h"


namespace BitFunnel
{
    DocumentHistogram::DocumentHistogram(std::istream & input)
        : m_documentCount(0)
    {
        CsvTsv::CsvTableParser parser(input);
        CsvTsv::TableReader reader(parser);

        CsvTsv::InputColumn<uint64_t> postingCount(
            "Postings",
            "Total postings in a document.");
        CsvTsv::InputColumn<uint64_t> documentCount(
            "Count",
            "Number of documents that have a given posting count.");

        reader.DefineColumn(postingCount);
        reader.DefineColumn(documentCount);

        reader.ReadPrologue();

        while (!reader.AtEOF())
        {
            reader.ReadDataRow();

            m_entries.push_back(std::make_pair(postingCount,
                                               documentCount));
            m_documentCount += documentCount;
        }

        // TODO: Seems like this would read past EOF.
        reader.ReadEpilogue();
    }


    size_t DocumentHistogram::GetEntryCount() const
    {
        return m_entries.size();
    }


    double DocumentHistogram::GetTotalDocumentCount() const
    {
        return static_cast<double>(m_documentCount);
    }


    size_t DocumentHistogram::GetPostingCount(size_t index) const
    {
        return m_entries[index].first;
    }


    double DocumentHistogram::GetDocumentCount(size_t index) const
    {
        return static_cast<double>(m_entries[index].second);
    }
}
