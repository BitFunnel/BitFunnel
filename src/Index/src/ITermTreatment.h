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

#include <iosfwd>                       // std::ostream parameter.

#include "BitFunnel/BitFunnelTypes.h"   // Rank parameter.
#include "BitFunnel/IInterface.h"       // Inherits from IInterface.
#include "BitFunnel/RowId.h"            // RowIndex parameter.
#include "BitFunnel/Term.h"             // Term parameter.


namespace BitFunnel
{
    class RowConfiguration;


    //*************************************************************************
    //
    // ITermTreatment
    //
    // An abstract base class or interface for classes mapping Terms to their
    // RowConfiguration.
    //
    // This mapping is used during TermTable construction to determine the
    // number of rows required in each RowTable. It is used during query
    // processing to determine the specific hash functions used to identify
    // each row corresponding to a term.
    //
    // ITermTreatment was designed as an interface to allow for pluggable
    // mappings. Some examples include
    //   Naive treatment where every term gets a single, rank 0 private row.
    //   Frequency based treatment where rarer terms get more rows.
    //   Sophisticated treatment where terms get rows of varying rank.
    //   Table-based treatment where table was built by Machine Learning.
    //
    //*************************************************************************
    class ITermTreatment : public IInterface
    {
    public:
        // Returns the RowConfiguration describing how a particular term
        // should be represented in the index.
        virtual RowConfiguration GetTreatment(Term term) const = 0;
    };


    //*************************************************************************
    //
    // RowConfiguration
    //
    // Provides a small collection of (Rank, RowCount, IsPrivate) tuples used
    // to describe how a Term will be represented in the Index.
    //
    // For example,
    //   A term with a single private, rank 0 row would be represented by the
    //   RowConfiguration {(0, 1, true)}.
    //
    //   A term with one shared rank 0 row and 4 shared rank 3 rows would be
    //   represented by the RowConfiguration {(0, 1, false), (3, 4, false)}.
    //
    // RowConfiguration is designed to be a small value type that can be passed
    // in a register. As such, it is implemented as a packed array of bit
    // fields.
    //
    // In order to fit into a quadword, a RowConfiguration is limited to eight
    // entries, each of which consists of a 3-bit rank, a 4 bit row count, and
    // one bit indicating whether the row(s) are private.
    //
    // DESIGN RATIONALE. RowConfiguration is used in the query pipeline when
    // processing adhoc terms. Since the query pipeline is performance
    // critical, RowConfiguration was designed as a small value type that could
    // be constructed and manipulated without heap allocations.
    //
    //*************************************************************************
    class RowConfiguration
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

            void Write(std::ostream& output) const;

        private:
            friend class RowConfiguration;

            Entry(uint8_t rawData);

            // Packed data field.
            // | 1-bit is private | 3-bit rank | 4-bit row count |
            uint8_t m_data;
        };

        class iterator : public std::iterator<std::input_iterator_tag, RowConfiguration::Entry>
        {
        public:
            bool operator!=(iterator const & other) const;
            iterator& operator++();
            Entry operator*() const;

        private:
            friend class RowConfiguration;

            iterator(uint64_t data);
            uint64_t m_data;
        };

        RowConfiguration();

        void push_front(Entry entry);

        iterator begin() const;
        iterator end() const;

        void Write(std::ostream& output) const;

    private:
        static const size_t c_log2MaxRowCount = 4;
        static const size_t c_maxRowCount = (1ull << c_log2MaxRowCount) - 1;

        uint64_t m_data;
    };
}
