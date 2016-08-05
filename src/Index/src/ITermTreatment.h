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

#include "BitFunnel/BitFunnelTypes.h"   // Rank parameter.
#include "BitFunnel/IInterface.h"       // Inherits from IInterface.
#include "BitFunnel/RowId.h"            // RowIndex parameter.
#include "BitFunnel/Term.h"             // Term parameter.


namespace BitFunnel
{
    // Design goals:
    //   Use in query pipeline ==> fast, no memory allocations.

    // Issues:
    //   How is the class configured for density, SNR, available ranks?
    //   Is an instance created as each term is processed, or are a fixed number
    //   of instances associated with classes of terms created at startup?

    class RowConfiguration // : public std::iterator<std::input_iterator_tag, RowConfiguration::Entry>
    {
    public:
        class Entry
        {
        public:
            Entry(Rank rank, RowIndex rowCount, bool isPrivate);

            Rank GetRank() const;
            RowIndex GetRowCount() const;
            bool IsPrivate() const;

            bool operator==(Entry const & other) const;

        private:
            friend class RowConfiguration;

            Entry(uint8_t rawData);

            uint8_t m_data;
        };

        typedef RowConfiguration iterator;

        RowConfiguration();

        void push_front(Entry entry);

        iterator begin() const;
        iterator end() const;

        bool operator!=(RowConfiguration const & other) const;
        RowConfiguration& operator++();
        Entry operator*() const;

    private:
        uint64_t m_data;
    };


    class ITermTreatment : public IInterface
    {
        virtual RowConfiguration GetTreatment(Term term) const = 0;
    };
}
