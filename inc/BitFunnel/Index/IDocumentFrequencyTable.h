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

#pragma once

#include <vector>                       // std::vector return value.

#include "BitFunnel/IInterface.h"       // Base class.
#include "BitFunnel/Term.h"             // Term member.


namespace BitFunnel
{
    class ITermToText;

    //*************************************************************************
    //
    // IDocumentFrequencyTable
    //
    // Abstract base class or interface for classes implementing a container
    // of document frequency records, each of which contains a Term and its
    // frequency in a corpus.
    //
    //*************************************************************************
    class IDocumentFrequencyTable : public IInterface
    {
    public:
        class Entry;

        // Sorts the entries by descending frequency then writes to a stream.
        // If the optional termToText parameter is not nullptr, the file will
        // contain a "text" column with the text for each term, if available
        // via the ITermToText. Note: method is not const because it sorts
        // the entries.
        virtual void Write(std::ostream & output,
                           ITermToText const * termToText) = 0;

        // Adds an Entry to the table. Note that this method does not guard
        // against duplicate Term::Hash values and it does not enforce any
        // ordering on the frequencies. Entries are sorted on write.
        virtual void AddEntry(Entry const & entry) = 0;

        // Returns the Entry corresponding a specific index.
        virtual Entry const & operator[](size_t index) const = 0;

        // Iterator methods.
        virtual std::vector<Entry>::const_iterator begin() const = 0;
        virtual std::vector<Entry>::const_iterator end() const = 0;

        virtual size_t size() const = 0;

        class Entry
        {
        public:
            Entry(Term term, double frequency)
              : m_term(term),
                m_frequency(frequency)
            {
            }

            Term GetTerm() const
            {
                return m_term;
            }

            double GetFrequency() const
            {
                return m_frequency;
            }

            bool operator==(Entry const & other) const
            {
                return
                    m_term == other.m_term &&
                    m_frequency == other.m_frequency;
            }

        private:
            Term m_term;
            double m_frequency;
        };
    };
}
