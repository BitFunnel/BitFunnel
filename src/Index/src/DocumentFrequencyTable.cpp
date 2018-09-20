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

#include <algorithm>    // std::sort()
#include <istream>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/ITermToText.h"
#include "CsvTsv/Csv.h"
#include "DocumentFrequencyTable.h"
#include "TermToText.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods
    //
    //*************************************************************************
    std::unique_ptr<IDocumentFrequencyTable>
        Factories::CreateDocumentFrequencyTable(std::istream& input)
    {
        return
            std::unique_ptr<IDocumentFrequencyTable>(
                new DocumentFrequencyTable(input));
    }


    //*************************************************************************
    //
    // DocumentFrequencyTable
    //
    //*************************************************************************
    DocumentFrequencyTable::DocumentFrequencyTable()
    {
    }


    DocumentFrequencyTable::DocumentFrequencyTable(std::istream& input)
    {
        //
        // Read sorted entries from stream.
        //

        CsvTsv::CsvTableParser parser(input);
        CsvTsv::TableReader reader(parser);

        CsvTsv::InputColumn<Term::Hash> hash(
            "hash",
            "Term's raw hash.");
        hash.SetHexMode(true);

        // NOTE: Cannot use OutputColumn<Term::GramSize> because OutputColumn
        // does not implement a specialization for char.
        CsvTsv::InputColumn<unsigned> gramSize(
            "gramSize",
            "Term's gram size.");

        // NOTE: Cannot use OutputColumn<Term::StreamId> because OutputColumn
        // does not implement a specialization for char.
        CsvTsv::InputColumn<unsigned> streamId(
            "streamId",
            "Term's stream id.");

        CsvTsv::InputColumn<double> frequency(
            "frequency",
            "Term's frequency.");

        CsvTsv::InputColumn<std::string> text(
            "text",
            "Term's text.");

        reader.DefineColumn(hash);
        reader.DefineColumn(gramSize);
        reader.DefineColumn(streamId);
        reader.DefineColumn(frequency);
        reader.DefineColumn(text);

        reader.ReadPrologue();

        while (!reader.AtEOF())
        {
            reader.ReadDataRow();

            Term term(hash,
                      static_cast<Term::StreamId>(streamId),
                      static_cast<Term::GramSize>(gramSize));

            if (m_entries.size() > 0 && m_entries.back().GetFrequency() < frequency)
            {
                RecoverableError
                    error("DocumentFrequencyTable: expect non-increasing frequencies.");
                throw error;
            }

            m_entries.push_back(Entry(term, frequency));
        }

        // TODO: Seems like this would read past EOF.
        reader.ReadEpilogue();
    }


    void DocumentFrequencyTable::Write(std::ostream & output,
                                       ITermToText const * termToText)
    {
        //
        // Sort entries by descending frequency.
        //

        struct
        {
            bool operator() (Entry a, Entry b)
            {
                // Sorts by decreasing frequency.
                return a.GetFrequency() > b.GetFrequency();
            }
        } compare;

        // Sort document frequency records by decreasing frequency.
        std::sort(m_entries.begin(), m_entries.end(), compare);


        //
        // Write sorted entries to stream.
        //

        CsvTsv::CsvTableFormatter formatter(output);
        CsvTsv::TableWriter writer(formatter);

        CsvTsv::OutputColumn<Term::Hash> hash(
            "hash",
            "Term's raw hash.");
        hash.SetHexMode(true);

        // NOTE: Cannot use OutputColumn<Term::GramSize> because OutputColumn
        // does not implement a specialization for char.
        CsvTsv::OutputColumn<unsigned> gramSize(
            "gramSize",
            "Term's gram size.");

        // NOTE: Cannot use OutputColumn<Term::StreamId> because OutputColumn
        // does not implement a specialization for char.
        CsvTsv::OutputColumn<unsigned> streamId(
            "streamId",
            "Term's stream id.");

        CsvTsv::OutputColumn<double> frequency(
            "frequency",
            "Term's frequency.");

        CsvTsv::OutputColumn<std::string> text(
            "text",
            "Term's text.");

        writer.DefineColumn(hash);
        writer.DefineColumn(gramSize);
        writer.DefineColumn(streamId);
        writer.DefineColumn(frequency);
        writer.DefineColumn(text);

        writer.WritePrologue();

        for (auto & entry : m_entries)
        {
            Term const & term = entry.GetTerm();
            hash = term.GetRawHash();
            gramSize = term.GetGramSize();
            streamId = term.GetStream();
            frequency = entry.GetFrequency();

            if (termToText != nullptr)
            {
                text = termToText->Lookup(term.GetRawHash());
            }
            else
            {
                text = std::string("");
            }

            writer.WriteDataRow();
        }

        writer.WriteEpilogue();
    }


    void DocumentFrequencyTable::AddEntry(Entry const & entry)
    {
        m_entries.push_back(entry);
    }


    DocumentFrequencyTable::Entry const & DocumentFrequencyTable::operator[](size_t index) const
    {
        return m_entries[index];
    }


    std::vector<DocumentFrequencyTable::Entry>::const_iterator DocumentFrequencyTable::begin() const
    {
        return m_entries.begin();
    }


    std::vector<DocumentFrequencyTable::Entry>::const_iterator DocumentFrequencyTable::end() const
    {
        return m_entries.end();
    }


    size_t DocumentFrequencyTable::size() const
    {
        return m_entries.size();
    }
}
