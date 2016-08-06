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

#include <ostream>

#include "BitFunnel/Exceptions.h"
#include "ITermTreatment.h"         // TODO: place this file in the right location.


namespace BitFunnel
{
    static const unsigned c_maxRank = 7;
    static const unsigned c_maxRowCount = 15;

    //*************************************************************************
    //
    // RowConfiguration::Entry
    //
    //*************************************************************************
    RowConfiguration::Entry::Entry(Rank rank, RowIndex rowCount, bool isPrivate)
    {
        if (rank > c_maxRank || rowCount > c_maxRowCount)
        {
            RecoverableError error("RowConfiguration::Entry::Entry: parameter value out of range.");
            throw error;
        }

        if (rowCount == 0)
        {
            RecoverableError error("RowConfiguration::Entry::Entry: row count must be greater than zero.");
            throw error;
        }

        m_data = static_cast<uint8_t>(rowCount | (rank << 4) | (isPrivate ? 0x80 : 0));
    }


    RowConfiguration::Entry::Entry(uint8_t data)
        : m_data(data)
    {
    }


    Rank RowConfiguration::Entry::GetRank() const
    {
        return (m_data >> 4) & 0x7;
    }


    RowIndex RowConfiguration::Entry::GetRowCount() const
    {
        return m_data & 0xf;
    }


    bool RowConfiguration::Entry::IsPrivate() const
    {
        return (m_data & 0x80) != 0;
    }


    bool RowConfiguration::Entry::operator==(Entry const & other) const
    {
        return m_data == other.m_data;
    }


    void RowConfiguration::Entry::Write(std::ostream& output) const
    {
        output
            << "(" << GetRank()
            << ", " << GetRowCount()
            << ", " << (IsPrivate() ? "private)" : "shared)");
    }


    //*************************************************************************
    //
    // RowConfiguration::iterator
    //
    //*************************************************************************
    RowConfiguration::iterator::iterator(uint64_t data)
        : m_data(data)
    {
    }


    bool RowConfiguration::iterator::operator!=(RowConfiguration::iterator const & other) const
    {
        return m_data != other.m_data;
    }


    RowConfiguration::iterator& RowConfiguration::iterator::operator++()
    {
        if (m_data == 0)
        {
            RecoverableError error("RowConfiguration::operator++: no more entries.");
            throw error;
        }
        else
        {
            m_data >>= 8;
            return *this;
        }
    }


    RowConfiguration::Entry RowConfiguration::iterator::operator*() const
    {
        if (m_data == 0)
        {
            RecoverableError error("RowConfiguration::operator*: no more entries.");
            throw error;
        }

        return Entry(static_cast<uint8_t>(m_data & 0xff));
    }



    //*************************************************************************
    //
    // RowConfiguration
    //
    //*************************************************************************
    RowConfiguration::RowConfiguration()
        : m_data(0)
    {
    }


    void RowConfiguration::push_front(Entry entry)
    {
        if ((m_data & 0xff00000000000000) != 0)
        {
            RecoverableError error("RowConfiguration::push_back: out of space.");
            throw error;
        }

        // Ensure that the entry doesn't have the same rank as an existing entry.
        // The restriction that no rank appears more than once simplified the bin
        // packing algorithm in the term table build which must ensure that all
        // rows for a term at a rank are distinct.
        for (auto existing : *this)
        {
            if (existing.GetRank() == entry.GetRank())
            {
                RecoverableError error("RowConfiguration::push_back: duplicate rank not allowed.");
                throw error;
            }
        }

        m_data <<= 8;
        m_data |= entry.m_data;
    }


    RowConfiguration::iterator RowConfiguration::begin() const
    {
        return iterator(m_data);
    }


    RowConfiguration::iterator RowConfiguration::end() const
    {
        return iterator(0ull);
    }


    void RowConfiguration::Write(std::ostream& output) const
    {
        output << "{ ";

        for (auto e : *this)
        {
            e.Write(output);
            output << " ";
        }

        output << "}";
    }
}
