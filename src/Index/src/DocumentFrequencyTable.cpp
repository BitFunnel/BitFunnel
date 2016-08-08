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

#include <cctype>                           // For isspace.
#include <istream>

#include "BitFunnel/Exceptions.h"
#include "DocumentFrequencyTable.h"


namespace BitFunnel
{
    static void SkipWhitespace(std::istream& input)
    {
        while(std::isspace(input.peek()))
        {
            input.get();
        }
    }


    static void Consume(std::istream& input, char expected)
    {
        char c;
        input >> c;
        if (c != expected)
        {
            FatalError error("DocumentFrequencyTable: bad input format.");
            throw error;
        }
    }


    DocumentFrequencyTable::DocumentFrequencyTable(std::istream& input)
    {
        while (input.peek() != EOF)
        {
            unsigned temp;

            Term::Hash rawHash;
            input >> std::hex >> rawHash;
            Consume(input, ',');

            Term::GramSize gramSize;
            input >> std::dec >> temp;
            gramSize = static_cast<Term::GramSize>(temp);
            Consume(input, ',');

            Term::StreamId streamId;
            input >> std::dec >> temp;
            streamId = static_cast<Term::StreamId>(temp);

            Consume(input, ',');

            double frequency;
            input >> frequency;

            const Term::IdfX10 maxIdf = 60;
            Term t(rawHash, streamId, Term::ComputeIdfX10(frequency, maxIdf));

            if (m_entries.size() > 0 && m_entries.back().GetFrequency() < frequency)
            {
                RecoverableError
                    error("DocumentFrequencyTable: expect non-increasing frequencies.");
                throw error;
            }

            m_entries.push_back(Entry(t, frequency));

            // Need to delete whitespace so that peeking for EOF doesn't get a
            // '\n'.
            SkipWhitespace(input);
        }
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
