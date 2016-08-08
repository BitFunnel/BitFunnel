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
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/RowId.h"


namespace BitFunnel
{
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4267)
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
    RowId::RowId(ShardId shard, Rank rank, RowIndex index)
        : m_shard(shard), m_rank(rank), m_index(index)
    {
        if (index > c_maxRowIndexValue)
        {
            throw RecoverableError("RowId::RowId(): Row index out of range.");
        }

        if (rank > c_maxRankValue)
        {
            throw RecoverableError("RowId::RowId(): Rank out of range.");
        }

        if (shard > c_maxShardIdValue)
        {
            throw RecoverableError("RowId::RowId(): ShardId out of range.");
        }
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif


    RowId::RowId(const RowId& other)
        : m_shard(other.m_shard), m_rank(other.m_rank), m_index(other.m_index)
    {
    }


    RowId::RowId(uint32_t packedRepresentation)
    {
        m_index = packedRepresentation;
        packedRepresentation >>= c_log2MaxRowIndexValue;
        m_rank = packedRepresentation;
        packedRepresentation >>= c_log2MaxRankValue;
        m_shard = packedRepresentation;
    }


    uint32_t RowId::GetPackedRepresentation() const
    {
        uint32_t packedRepresentation = m_shard;
        packedRepresentation <<= c_log2MaxRankValue;
        packedRepresentation |= m_rank;
        packedRepresentation <<= c_log2MaxRowIndexValue;
        packedRepresentation |= m_index;
        return packedRepresentation;
    }


    Rank RowId::GetRank() const
    {
        return m_rank;
    }


    ShardId RowId::GetShard() const
    {
        return m_shard;
    }


    RowIndex RowId::GetIndex() const
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


    // bool RowId::IsValid() const
    // {
    //     // DESIGN NOTE: this needs to change once we pass the query plan. We
    //     // wanted to use the same JIT'ed code on each shard. But not all shards
    //     // have the same number of rows. The way that's done is by duplicating
    //     // rows so that every shard ends up as long as the longest
    //     // shard. IsValid was used to find the boundary where things need to be
    //     // duplicated.

    //     // It's not clear that we need to do this for the ported BitFunnel. We
    //     // should conduct the experiment before really porting this over because
    //     // it adds a significant amount of complexity.
    //     return true;
    // }


    // TODO: Who calls this?
    unsigned RowId::GetPackedRepresentationBitCount()
    {
        return c_log2MaxShardIdValue
               + c_log2MaxRankValue
               + c_log2MaxRowIndexValue;
    }
}
