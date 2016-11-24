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


// TODO: should this file be in Index?

#include <algorithm>

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
// #include "BitFunnel/Index/IShardIndex.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Plan/Factories.h"
#include "IPlanRows.h"
#include "IRowSet.h"
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
