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

#include <cctype>  // For isspace.
#include <istream>

#include "DocumentFrequencyTable.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    void SkipWhitespace(std::istream& input)
    {
        while(std::isspace(input.peek()))
        {
            input.get();
        }
    }


    DocumentFrequencyTable::DocumentFrequencyTable(std::istream& input)
    {
        while (input.peek() != EOF)
        {
            char comma;
            Term t(input);
            input >> comma;
            LogAssertB(comma == ',', "Bad input format.");
            double frequency;
            input >> frequency;
            m_entries.push_back(std::make_pair(t, frequency));

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
        return m_entries.begin();
    }


    size_t DocumentFrequencyTable::size() const
    {
        return m_entries.size();
    }
}
