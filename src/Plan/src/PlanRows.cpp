#include "stdafx.h"

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "BitFunnel/Factories.h"
#include "BitFunnel/IIndexConfiguration.h"
#include "BitFunnel/ITermTableCollection.h"
#include "BitFunnel/StreamUtilities.h"
#include "PlanRows.h"


namespace BitFunnel
{
    IPlanRows& Factories::CreatePlanRows(IInputStream& input,
                                         const IIndexConfiguration& index,
                                         Allocators::IAllocator& allocator)
    {
        return *new (allocator.Allocate(sizeof(PlanRows))) PlanRows(input, index);
    }


    //*************************************************************************
    //
    // PlanRows
    //
    //*************************************************************************
    PlanRows::PlanRows(const IIndexConfiguration& index)
        : m_index(index)
    {
    }


    PlanRows::PlanRows(IInputStream& stream, const IIndexConfiguration& index)
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
        return static_cast<unsigned>(m_index.GetShardCount());
    }


    unsigned PlanRows::GetRowCount() const
    {
        return m_rows.GetSize();
    }


    const ITermTable& PlanRows::GetTermTable(ShardId shard) const
    {
        return *m_index.GetTermTables().GetTermTable(shard);
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
        StreamUtilities::ReadArray(stream, m_rowIds, c_maxShardCount);
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
        LogAssertB(shard < c_maxShardCount);
        return m_rowIds[shard];
    }


    RowId const & PlanRows::Entry::operator[](ShardId shard) const
    {
        LogAssertB(shard < c_maxShardCount);
        return m_rowIds[shard];
    }


    void PlanRows::Entry::Write(std::ostream& stream) const
    {
        StreamUtilities::WriteField(stream, m_id);
        StreamUtilities::WriteField(stream, m_rank);
        StreamUtilities::WriteArray(stream, m_rowIds, c_maxShardCount);
    }
}
