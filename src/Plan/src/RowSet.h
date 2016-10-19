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


#pragma once

#include "BitFunnel/Plan/IRowSet.h"
#include "BitFunnel/NonCopyable.h"

namespace BitFunnel
{
    class IAllocator;
    // class Context;
    // class IIndexData;
    class ISimpleIndex;

    class RowSet : public IRowSet, NonCopyable
    {
    public:
        RowSet(ISimpleIndex const & index,
               const IPlanRows& planRows,
               IAllocator& allocator);

        //
        // IRowSet API.
        //
        // In the current implementation, non-DDR rows are not supported
        virtual void LoadRows(//Context const & context,
                              //IRowsAvailable& rowsAvailable
                              ) override;
        virtual ShardId GetShardCount() const override;
        virtual unsigned GetRowCount() const override;
        virtual ptrdiff_t const * GetRowOffsets(ShardId shard) const override;

    private:
        //
        // Constructor parameters
        //
        const IPlanRows& m_planRows;
        // IIndexData const & m_indexData;
        ISimpleIndex const & m_index;
        // IAllocator& m_allocator;

        ptrdiff_t ** m_rows;
    };
}
