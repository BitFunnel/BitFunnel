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
#include <memory>

#include "BitFunnel/ITermTable.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/TermInfo.h"
#include "LoggerInterfaces/Logging.h"
#include "RowTableDescriptor.h"


namespace BitFunnel
{
    RowTableDescriptor::RowTableDescriptor(DocIndex capacity,
                                           RowIndex rowCount,
                                           Rank rank,
                                           ptrdiff_t rowTableBufferOffset)
        : m_capacity(capacity),
          m_rowCount(rowCount),
          m_rank(rank),
          m_bufferOffset(rowTableBufferOffset),
          m_bytesPerRow(Row::BytesInRow(capacity, rank))
    {
        // Make sure capacity is properly rounded already.
        // TODO: fix.
        // LogAssertB(capacity == Row::DocumentsInRank0Row(capacity),
        //            "capacity not evenly rounded.");

        // // Make sure offset of this RowTable is properly aligned.
        // LogAssertB((rowTableBufferOffset % c_rowTableByteAlignment) == 0);
    }


    RowTableDescriptor::RowTableDescriptor(RowTableDescriptor const & other)
        : m_capacity(other.m_capacity),
          m_rowCount(other.m_rowCount),
          m_rank(other.m_rank),
          m_bufferOffset(other.m_bufferOffset),
          m_bytesPerRow(other.m_bytesPerRow)
    {
    }


    void RowTableDescriptor::Initialize(void* sliceBuffer, ITermTable const & termTable) const
    {
        char* const rowTableBuffer = reinterpret_cast<char*>(sliceBuffer) + m_bufferOffset;
        memset(rowTableBuffer, 0, GetBufferSize(m_capacity, m_rowCount, m_rank));

        // The "match-all" row needs to be initialized differently.
        TermInfo termInfo(ITermTable::GetMatchAllTerm(), termTable);
        LogAssertB(termInfo.MoveNext(),""); // TODO: error message.

        const RowId matchAllRowId = termInfo.Current();
        LogAssertB(!termInfo.MoveNext(), ""); // TODO: error message.

        if (matchAllRowId.GetRank() == m_rank)
        {
            // Fill up the match-all row with all ones.
            uint64_t * rowData = GetRowData(sliceBuffer, matchAllRowId.GetIndex());
            memset(rowData, 0xFF, m_bytesPerRow);
        }
    }


    uint64_t RowTableDescriptor::GetBit(void* sliceBuffer,
                                    RowIndex rowIndex,
                                    DocIndex docIndex) const
    {
        uint64_t* const row = GetRowData(sliceBuffer, rowIndex);
        const size_t offset = QwordPositionFromDocIndex(docIndex);

        // Note that the offset here is shifted left by 6.
        // return _bittest64(row + (offset >> 6), offset & 0x3F);

        uint64_t bitPos = offset & 0x3F;
        uint64_t bitMask = 1ull << bitPos;
        uint64_t maskedVal = *(row + offset) & bitMask;
        // TODO: check if the usage of this actually requires shifting the bit
        // back down.
        return maskedVal >> bitPos;
    }


    void RowTableDescriptor::SetBit(void* sliceBuffer,
                                    RowIndex rowIndex,
                                    DocIndex docIndex) const
    {
        uint64_t* const row = GetRowData(sliceBuffer, rowIndex);
        const size_t offset = QwordPositionFromDocIndex(docIndex);

        // Note that the offset here is shifted left by 6.
        // _interlockedbittestandset64(row + (offset >> 6), offset & 0x3F);

        uint64_t bitPos = offset & 0x3F;
        uint64_t bitMask = 1ull << bitPos;
        uint64_t newVal = *(row + offset) | bitMask;
        *(row + offset) = newVal;
    }


    void RowTableDescriptor::ClearBit(void* sliceBuffer,
                                      RowIndex rowIndex,
                                      DocIndex docIndex) const
    {
        uint64_t* const row = GetRowData(sliceBuffer, rowIndex);
        const size_t offset = QwordPositionFromDocIndex(docIndex);

        // Note that the offset here is shifted left by 6.
        // _interlockedbittestandreset64(row + (offset >> 6), offset & 0x3F);

        uint64_t bitPos = offset & 0x3F;
        uint64_t bitMask = ~(1ull << bitPos);
        uint64_t newVal = *(row + offset) & bitMask;
        *(row + offset) = newVal;
    }


    ptrdiff_t RowTableDescriptor::GetRowOffset(RowIndex rowIndex) const
    {
        return m_bufferOffset + rowIndex * m_bytesPerRow;
    }


    /* static */
    size_t RowTableDescriptor::GetBufferSize(DocIndex capacity,
                                             RowIndex rowCount,
                                             Rank rank)
    {
        // Make sure capacity is properly rounded already.
        // TODO: fix.
        // LogAssertB(capacity == Row::DocumentsInRank0Row(capacity),
        //            "capacity not evenly rounded.");

        return static_cast<unsigned>(Row::BytesInRow(capacity, rank) * rowCount);
    }


    uint64_t* RowTableDescriptor::GetRowData(void* sliceBuffer,
                                            RowIndex rowIndex) const
    {
        char* rowData = reinterpret_cast<char*>(sliceBuffer) + GetRowOffset(rowIndex);
        return reinterpret_cast<uint64_t*>(rowData);
    }


    size_t RowTableDescriptor::QwordPositionFromDocIndex(DocIndex docIndex) const
    {
        LogAssertB(docIndex < m_capacity, "docIndex out of range");

        // Shifting by 6 gives the rank0Word.
        // Shifting by an additional m_rank gives the rankRWord.
        return docIndex >> (6 + m_rank);
    }


    size_t RowTableDescriptor::BitPositionFromDocIndex(DocIndex docIndex) const
    {
        LogAssertB(docIndex < m_capacity, "docIndex out of range");

        size_t rank0Word = docIndex >> 6;
        size_t rankRWord = rank0Word >> m_rank;
        // TODO: check if compiler is smart enough to avoid doing a % here.
        size_t bitInWord = docIndex % 64;
        return (rankRWord << 6) + bitInWord;
    }
}
