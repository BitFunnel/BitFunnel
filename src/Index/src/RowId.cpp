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


#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/RowId.h"


namespace BitFunnel
{
    RowId::RowId()
    //        : m_tier(static_cast<unsigned>(InvalidTier))
    {
    }


#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4267)
#endif
    RowId::RowId(size_t shard, size_t rank, size_t index)
        : m_shard(shard), m_rank(rank), m_index(index)
    {
        LogAssertB(index <= c_maxRowIndexValue, "Row index out of range.");
        // TODO: assert that other values are in range.
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif


    RowId::RowId(const RowId& other)
        : m_shard(other.m_shard), m_rank(other.m_rank), m_index(other.m_index)
    {
    }


    RowId::RowId(uint64_t packedRepresentation)
    {
        m_index = packedRepresentation;
        packedRepresentation >>= c_bitsOfIndex;
        m_rank = packedRepresentation;
        packedRepresentation >>= c_bitsOfRank;
        m_shard = packedRepresentation;
    }


    uint64_t RowId::GetPackedRepresentation() const
    {
        uint64_t packedRepresentation = m_shard;
        packedRepresentation <<= c_bitsOfRank;
        packedRepresentation |= m_rank;
        packedRepresentation <<= c_bitsOfIndex;
        packedRepresentation |= m_index;
        return packedRepresentation;
    }


    size_t RowId::GetRank() const
    {
        return m_rank;
    }


    size_t RowId::GetShard() const
    {
        return m_shard;
    }


    size_t RowId::GetIndex() const
    {
        return m_index;
    }


    bool RowId::operator==(const RowId& other) const
    {
        return m_index == other.GetIndex()
            && m_rank == other.GetRank()
            && m_shard == other.GetShard();
    }


    bool RowId::operator!=(const RowId& other) const
    {
        return !(*this == other);
    }


    bool RowId::operator<(const RowId& other) const
    {
        if (m_shard != other.m_shard)
        {
            return m_shard < other.m_shard;
        }

        if (m_rank != other.m_rank)
        {
            return m_rank < other.m_rank;
        }

        return m_index < other.m_index;
    }


    bool RowId::IsValid() const
    {
        // DESIGN NOTE: this needs to change once we pass the query plan. We
        // wanted to use the same JIT'ed code on each shard. But not all shards
        // have the same number of rows. The way that's done is by duplicating
        // rows so that every shard ends up as long as the longest
        // shard. IsValid was used to find the boundary where things need to be
        // duplicated.

        // It's not clear that we need to do this for the ported BitFunnel. We
        // should conduct the experiment before really porting this over because
        // it adds a significant amount of complexity.
        return true;
    }


    unsigned RowId::GetPackedRepresentationBitCount()
    {
        return c_bitsOfShard
               + c_bitsOfRank
               + c_bitsOfIndex;
    }
}
