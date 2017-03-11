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


#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "BitFunnel/Plan/Factories.h"
#include "PlanRows.h"


namespace BitFunnel
{
    IPlanRows& Factories::CreatePlanRows(IInputStream& input,
                                         const ISimpleIndex& index,
                                         IAllocator& allocator)
    {
        return *new (allocator.Allocate(sizeof(PlanRows))) PlanRows(input, index);
    }


    //*************************************************************************
    //
    // PlanRows
    //
    //*************************************************************************
    PlanRows::PlanRows(const ISimpleIndex& index)
        : m_index(index)
    {
    }


    PlanRows::PlanRows(IInputStream& stream, const ISimpleIndex& index)
        : m_index(index)
    {
        const unsigned size = StreamUtilities::ReadField<unsigned>(stream);

        for (unsigned i = 0; i < size; ++i)
        {
            m_rows.PushBack(Entry(stream));
        }
    }


    PlanRows::~PlanRows()
    {
    }


    ShardId PlanRows::GetShardCount() const
    {
        // return static_cast<unsigned>(m_index.GetShardCount());
        return m_index.GetIngestor().GetShardCount();
    }


    unsigned PlanRows::GetRowCount() const
    {
        return m_rows.GetSize();
    }


    const ITermTable& PlanRows::GetTermTable(ShardId shard) const
    {
        return m_index.GetTermTable(shard);
    }


    bool PlanRows::IsFull() const
    {
        return (m_rows.GetSize() >= GetRowCountLimit());
    }


    // Add a row to the PlanRows.
    AbstractRow PlanRows::AddRow(Rank rank)
    {
        m_rows.PushBack(m_rows.GetSize(), rank);
        return AbstractRow(m_rows.GetSize() - 1, rank, false);
    }


    const RowId& PlanRows::PhysicalRow(ShardId shard, unsigned id) const
    {
        return m_rows[id][shard];
    }


    RowId& PlanRows::PhysicalRow(ShardId shard, unsigned id)
    {
        return m_rows[id][shard];
    }


    void PlanRows::Write(std::ostream& stream) const
    {
        StreamUtilities::WriteField(stream, m_rows.GetSize());

        for (unsigned i = 0; i < m_rows.GetSize(); ++i)
        {
            m_rows[i].Write(stream);
        }
    }


    unsigned PlanRows::GetRowCountLimit() const
    {
        return c_maxRowsPerQuery;
    }


    PlanRows::Entry::Entry(unsigned id, Rank rank)
        : m_id(id),
          m_rank(rank)
    {
    }


    PlanRows::Entry::Entry(IInputStream& stream)
    {
        m_id = StreamUtilities::ReadField<unsigned>(stream);
        m_rank = StreamUtilities::ReadField<Rank>(stream);
        StreamUtilities::ReadArray(stream, m_rowIds, c_maxShardIdCount);
    }


    unsigned PlanRows::Entry::GetId() const
    {
        return m_id;
    }


    Rank PlanRows::Entry::GetRank() const
    {
        return m_rank;
    }


    RowId& PlanRows::Entry::operator[](ShardId shard)
    {
        LogAssertB(shard < c_maxShardIdCount, "ShardId overflow.");
        return m_rowIds[shard];
    }


    RowId const & PlanRows::Entry::operator[](ShardId shard) const
    {
        LogAssertB(shard < c_maxShardIdCount, "ShardId overflow.");
        return m_rowIds[shard];
    }


    void PlanRows::Entry::Write(std::ostream& stream) const
    {
        StreamUtilities::WriteField(stream, m_id);
        StreamUtilities::WriteField(stream, m_rank);
        StreamUtilities::WriteArray(stream, m_rowIds, c_maxShardIdCount);
    }
}
