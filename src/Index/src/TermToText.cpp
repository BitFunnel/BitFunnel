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

#include "BitFunnel/Index/Factories.h"
#include "CsvTsv/Csv.h"
#include "TermToText.h"


namespace BitFunnel
{
    std::unique_ptr<ITermToText> Factories::CreateTermToText(std::istream & input)
    {
        return std::unique_ptr<ITermToText>(new TermToText(input));
    }


    std::unique_ptr<ITermToText> Factories::CreateTermToText()
    {
        return std::unique_ptr<ITermToText>(new TermToText());
    }


    TermToText::TermToText()
    {
    }


    TermToText::TermToText(std::istream & input)
    {
        CsvTsv::CsvTableParser parser(input);
        CsvTsv::TableReader reader(parser);

        CsvTsv::InputColumn<uint64_t> hash(
            "hash",
            "Term's raw hash.");
        hash.SetHexMode(true);

        CsvTsv::InputColumn<std::string> text(
            "text",
            "Term's text.");

        reader.DefineColumn(hash);
        reader.DefineColumn(text);
        reader.ReadPrologue();

        while (!reader.AtEOF())
        {
            reader.ReadDataRow();
            AddTerm(hash, text);
        }

        // TODO: Seems like this would read past EOF.
        void ReadEpilogue();
    }


    void TermToText::Write(std::ostream& output) const
    {
        CsvTsv::CsvTableFormatter formatter(output);
        CsvTsv::TableWriter writer(formatter);

        CsvTsv::OutputColumn<Term::Hash> hash(
            "hash",
            "Term's raw hash.");
        hash.SetHexMode(true);
        
        CsvTsv::OutputColumn<std::string> text(
            "text",
            "Term's text.");

        writer.DefineColumn(hash);
        writer.DefineColumn(text);
        writer.WritePrologue();

        for (auto & entry : m_termToText)
        {
            hash = entry.first;
            text = entry.second;
            writer.WriteDataRow();
        }

        writer.WriteEpilogue();
    }


    void TermToText::AddTerm(Term::Hash hash, std::string const & text)
    {
        auto it = m_termToText.find(hash);
        if (it == m_termToText.end())
        {
            m_termToText.insert(std::make_pair(hash, text));
        }
    }


    std::string const & TermToText::Lookup(Term::Hash hash) const
    {
        auto it = m_termToText.find(hash);
        if (it == m_termToText.end())
        {
            return m_emptyString;
        }
        else
        {
            return (*it).second;
        }
    }
}
