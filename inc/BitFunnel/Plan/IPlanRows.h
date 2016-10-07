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

#include <iosfwd>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Index/RowId.h"
#include "BitFunnel/Plan/AbstractRow.h"


namespace BitFunnel
{
    class ITermTable;

    class IPlanRows
    {
    public:
        virtual ~IPlanRows() {}

        virtual ShardId GetShardCount() const = 0;
        virtual unsigned GetRowCount() const = 0;

        virtual const ITermTable& GetTermTable(ShardId shard) const = 0;

        virtual bool IsFull() const = 0;
        virtual AbstractRow AddRow(Rank rank) = 0;

        virtual RowId const & PhysicalRow(ShardId shard, unsigned id) const = 0;
        virtual RowId& PhysicalRow(ShardId shard, unsigned id) = 0;

        // Writes the PlanRows data to a stream.
        virtual void Write(std::ostream& stream) const = 0;
    };
}
