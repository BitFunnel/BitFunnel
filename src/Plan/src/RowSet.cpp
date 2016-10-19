// TODO: should this file be in Index?

#include <algorithm>

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
// #include "BitFunnel/Index/IShardIndex.h"
#include "BitFunnel/Index/ISimpleIndex.h"
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
    IRowSet& Factories::CreateRowSet(ISimpleIndex const & index,
                                     IPlanRows const & planRows,
                                     IAllocator& allocator)
    {
        return *new (allocator.Allocate(sizeof(RowSet)))
                    RowSet(index, planRows, allocator);
    }


    //*************************************************************************
    //
    // RowSet
    //
    //*************************************************************************
    RowSet::RowSet(ISimpleIndex const & index,
                   IPlanRows const & planRows,
                   IAllocator& allocator)
        : m_planRows(planRows),
          m_index(index)
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
                          //IRowsAvailable& rowsAvailable
                          )
    {
        // For each shard, allocate an array of Row.
        for (ShardId shardId = 0; shardId < m_planRows.GetShardCount(); ++shardId)
        {
            // IShardIndex const & shard = m_indexData.GetShardIndex(shardId);
            IShard const & shard = m_index.GetIngestor().GetShard(shardId);
            for (unsigned i = 0; i < m_planRows.GetRowCount(); ++i)
            {
                const RowId rowId = m_planRows.PhysicalRow(shardId, i);

                // See https://github.com/BitFunnel/BitFunnel/issues/250.
                m_rows[shardId][i] = shard.GetRowOffset(rowId);
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
