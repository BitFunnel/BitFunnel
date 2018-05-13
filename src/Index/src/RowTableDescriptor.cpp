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

#include <cstring>

#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/Row.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "LoggerInterfaces/Check.h"
#include "LoggerInterfaces/Logging.h"
#include "RowTableDescriptor.h"

#ifdef _MSC_VER
#include <intrin.h>  // For _interlockedbittestandreset64, etc.
#endif


namespace BitFunnel
{
    RowTableDescriptor::RowTableDescriptor(DocIndex capacity,
                                           RowIndex rowCount,
                                           Rank rank,
                                           Rank maxRank,
                                           ptrdiff_t rowTableBufferOffset)
        : m_capacity(capacity),
          m_rowCount(rowCount),
          m_rank(rank),
          m_maxRank(maxRank),
          m_bufferOffset(rowTableBufferOffset),
          m_bytesPerRow(Row::BytesInRow(capacity, rank, maxRank))
    {
        // Make sure capacity is properly rounded already.
        // TODO: fix.
        // LogAssertB(capacity == Row::DocumentsInRank0Row(capacity),
        //            "capacity not evenly rounded.");

        // // Make sure offset of this RowTable is properly aligned.
        CHECK_EQ(rowTableBufferOffset % static_cast<ptrdiff_t>(c_rowTableByteAlignment), 0)
            << "incorrect buffer alignment.";
    }


    RowTableDescriptor::RowTableDescriptor(RowTableDescriptor const & other)
        : m_capacity(other.m_capacity),
          m_rowCount(other.m_rowCount),
          m_rank(other.m_rank),
          m_maxRank(other.m_maxRank),
          m_bufferOffset(other.m_bufferOffset),
          m_bytesPerRow(other.m_bytesPerRow)
    {
    }


    void RowTableDescriptor::Initialize(void* sliceBuffer,
                                        ITermTable const & termTable) const
    {
        char* const rowTableBuffer =
            reinterpret_cast<char*>(sliceBuffer) + m_bufferOffset;
        memset(rowTableBuffer,
               0,
               GetBufferSize(m_capacity, m_rowCount, m_rank, m_maxRank));

        // The "match-all" row needs to be initialized differently.
        RowIdSequence rows(ITermTable::GetMatchAllTerm(), termTable);

        auto it = rows.begin();
        if (it == rows.end())
        {
            RecoverableError
                error("RowTableDescriptor::Initialize: expected at least one row.");
            throw error;
        }

        const RowId row = *it;

        ++it;
        if (it != rows.end())
        {
            RecoverableError error("RowTableDescriptor::Initialize: expected no more than one row.");
            throw error;

        }

        if (row.GetRank() == m_rank)
        {
            // Fill up the match-all row with all ones.
            uint64_t * rowData = GetRowData(sliceBuffer, row.GetIndex());
            memset(rowData, 0xFF, m_bytesPerRow);
        }
    }


    RowIndex RowTableDescriptor::GetRowCount() const
    {
        return m_rowCount;
    }


    uint64_t RowTableDescriptor::GetBit(void const * sliceBuffer,
                                        RowIndex rowIndex,
                                        DocIndex docIndex) const
    {
        uint64_t const * row = GetRowData(sliceBuffer, rowIndex);
        const size_t offset = QwordPositionFromDocIndex(docIndex);
        uint64_t bitPos = docIndex & 0x3F;

#ifdef _MSC_VER
        return _bittest64(reinterpret_cast<long long const *>(row + offset), bitPos);
#else
        // TODO: benchmark this vs. btc instruction.
        uint64_t bitMask = 1ull << bitPos;
        uint64_t maskedVal = *(row + offset) & bitMask;
        return maskedVal;
#endif
    }


    void RowTableDescriptor::SetBit(void* sliceBuffer,
                                    RowIndex rowIndex,
                                    DocIndex docIndex) const
    {
        CHECK_LT(rowIndex, m_rowCount)
            << "rowIndex out of range.";
        uint64_t* const row = GetRowData(sliceBuffer, rowIndex);
        const size_t offset = QwordPositionFromDocIndex(docIndex);
        uint64_t bitPos = docIndex & 0x3F;


#ifdef _MSC_VER
        _interlockedbittestandset64(reinterpret_cast<long long *>(row + offset), bitPos);
#else
        // TODO: figure out if this should really be +m.
        asm("lock btsq %1, %0" : "+m" (*(row + offset)) : "r" (bitPos));
        // uint64_t bitMask = 1ull << bitPos;
        // uint64_t newVal = *(row + offset) | bitMask;
        // *(row + offset) = newVal;
#endif
    }


    void RowTableDescriptor::ClearBit(void* sliceBuffer,
                                      RowIndex rowIndex,
                                      DocIndex docIndex) const
    {
        uint64_t* const row = GetRowData(sliceBuffer, rowIndex);
        const size_t offset = QwordPositionFromDocIndex(docIndex);
        uint64_t bitPos = docIndex & 0x3F;

#ifdef _MSC_VER
        _interlockedbittestandreset64(reinterpret_cast<long long *>(row + offset), bitPos);
#else
        // TODO: figure out if this should really be +m.
        asm("lock btrq %1, %0" : "+m" (*(row + offset)) : "r" (bitPos));
        // uint64_t bitMask = ~(1ull << bitPos);
        // uint64_t newVal = *(row + offset) & bitMask;
        // *(row + offset) = newVal;
#endif
    }


    ptrdiff_t RowTableDescriptor::GetRowOffset(RowIndex rowIndex) const
    {
        // TODO: consider checking for overflow.
        return m_bufferOffset + static_cast<ptrdiff_t>(rowIndex * m_bytesPerRow);
    }


    /* static */
    size_t RowTableDescriptor::GetBufferSize(DocIndex capacity,
                                             RowIndex rowCount,
                                             Rank rank,
                                             Rank maxRank)
    {
        // Make sure capacity is properly rounded already.
        // TODO: fix.
        // LogAssertB(capacity == Row::DocumentsInRank0Row(capacity),
        //            "capacity not evenly rounded.");

        return static_cast<unsigned>(
            Row::BytesInRow(capacity, rank, maxRank) * rowCount);
    }


    uint64_t* RowTableDescriptor::GetRowData(void* sliceBuffer,
                                             RowIndex rowIndex) const
    {
        char* rowData =
            reinterpret_cast<char*>(sliceBuffer) +
            GetRowOffset(rowIndex);
        return reinterpret_cast<uint64_t*>(rowData);
    }


    uint64_t const *
        RowTableDescriptor::GetRowData(void const * sliceBuffer,
                                       RowIndex rowIndex) const
    {
        char const * rowData = 
            reinterpret_cast<char const *>(sliceBuffer) +
            GetRowOffset(rowIndex);
        return reinterpret_cast<uint64_t const *>(rowData);
    }


    size_t RowTableDescriptor::QwordPositionFromDocIndex(DocIndex docIndex) const
    {
        LogAssertB(docIndex < m_capacity, "docIndex out of range");

        // Shifting by 6 gives the rank0Word.
        // Shifting by an additional m_rank gives the rankRWord.
        return docIndex >> (6 + m_rank);
    }
}
