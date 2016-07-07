#include "stdafx.h"

#include <intrin.h>
#include <memory>                       // For memset.

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
        LogAssertB(capacity == Row::DocumentsInRank0Row(capacity));

        // Make sure offset of this RowTable is properly aligned.
        LogAssertB((rowTableBufferOffset % c_rowTableByteAlignment) == 0);
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
        LogAssertB(termInfo.MoveNext());

        const RowId matchAllRowId = termInfo.Current();
        LogAssertB(matchAllRowId.GetTier() == DDRTier);

        LogAssertB(!termInfo.MoveNext());

        if (matchAllRowId.GetRank() == m_rank)
        {
            // Fill up the match-all row with all ones.
            __int64 * rowData = GetRowData(sliceBuffer, matchAllRowId.GetIndex());
            memset(rowData, 0xFF, m_bytesPerRow);
        }
    }


    char RowTableDescriptor::GetBit(void* sliceBuffer,
                                    RowIndex rowIndex, 
                                    DocIndex docIndex) const
    {
        __int64* const row = GetRowData(sliceBuffer, rowIndex);
        const unsigned offset = BitPositionFromDocIndex(docIndex);

        return _bittest64(row + (offset >> 6), offset & 0x3F);
    }


    void RowTableDescriptor::SetBit(void* sliceBuffer, 
                                    RowIndex rowIndex, 
                                    DocIndex docIndex) const
    {
        __int64* const row = GetRowData(sliceBuffer, rowIndex);
        const unsigned offset = BitPositionFromDocIndex(docIndex);

        _interlockedbittestandset64(row + (offset >> 6), offset & 0x3F);
    }


    void RowTableDescriptor::ClearBit(void* sliceBuffer, 
                                      RowIndex rowIndex, 
                                      DocIndex docIndex) const
    {
        __int64* const row = GetRowData(sliceBuffer, rowIndex);
        const unsigned offset = BitPositionFromDocIndex(docIndex);
        _interlockedbittestandreset64(row + (offset >> 6), offset & 0x3F);
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
        LogAssertB(capacity == Row::DocumentsInRank0Row(capacity));

        return static_cast<unsigned>(Row::BytesInRow(capacity, rank) * rowCount);
    }


    __int64* RowTableDescriptor::GetRowData(void* sliceBuffer, 
                                            RowIndex rowIndex) const
    {
        char* rowData = reinterpret_cast<char*>(sliceBuffer) + GetRowOffset(rowIndex);
        return reinterpret_cast<__int64*>(rowData);
    }


    unsigned RowTableDescriptor::BitPositionFromDocIndex(DocIndex docIndex) const
    {
        LogAssertB(docIndex < m_capacity);

        int rank0Word = docIndex >> 6;
        int rankRWord = rank0Word >> m_rank;
        int bitInWord = docIndex % 64;
        return (rankRWord << 6) + bitInWord;
    }
}
