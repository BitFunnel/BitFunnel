// TODO: should this file be in Index?

#include <algorithm>

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/IIndexData.h"
#include "BitFunnel/Index/IShardIndex.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IPlanRows.h"
#include "BitFunnel/Plan/IRowSet.h"
#include "LoggerInterfaces/Logging.h"
#include "RowSet.h"


// #include "BitFunnel/Plan/IRowsAvailable.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************
    IRowSet& Factories::CreateRowSet(IIndexData const & indexData,
                                     IPlanRows const & planRows,
                                     IAllocator& allocator)
    {
        return *new (allocator.Allocate(sizeof(RowSet)))
                    RowSet(indexData, planRows, allocator);
    }


    //*************************************************************************
    //
    // RowSet
    //
    //*************************************************************************
    RowSet::RowSet(IIndexData const & indexData,
                   IPlanRows const & planRows,
                   IAllocator& allocator)
        : m_planRows(planRows),
          m_indexData(indexData)
          // m_allocator(allocator)
    {
        // Allocate one entry for each shard.
        m_rows = new (allocator.Allocate(sizeof(ptrdiff_t) * m_planRows.GetShardCount()))
                      ptrdiff_t*;
        // For each shard, allocate an array of Row.
        for (ShardId shard = 0; shard < m_planRows.GetShardCount(); ++shard)
        {
            m_rows[shard] = reinterpret_cast<ptrdiff_t*>(allocator.Allocate(sizeof(ptrdiff_t)
                                                                            * m_planRows.GetRowCount()));

        }
    }


    void RowSet::LoadRows(// Context const & context,
                          IRowsAvailable& /*rowsAvailable*/)
    {
        // For each shard, allocate an array of Row.
        for (ShardId shardId = 0; shardId < m_planRows.GetShardCount(); ++shardId)
        {
            IShardIndex const & shard = m_indexData.GetShardIndex(shardId);
            for (unsigned i = 0; i < m_planRows.GetRowCount(); ++i)
            {
                const RowId rowId = m_planRows.PhysicalRow(shardId, i);

                // MatcherRunner expects row offsets to be in quadwords.
                m_rows[shardId][i] = (shard.GetRowOffset(rowId) >> 3);
            }
        }

        // rowsAvailable.OnRowsAvailable(context, true);
    }


    ShardId RowSet::GetShardCount() const
    {
        return m_planRows.GetShardCount();
    }


    unsigned RowSet::GetRowCount() const
    {
        return m_planRows.GetRowCount();
    }


    ptrdiff_t const * RowSet::GetRowOffsets(ShardId shard) const
    {
        return m_rows[shard];
    }
}
