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
#include "BitFunnel/NonCopyable.h"
#include "FixedCapacityVector.h"
#include "IPlanRows.h"


namespace BitFunnel
{
    class ISimpleIndex;
    class IInputStream;


    class PlanRows : public IPlanRows, NonCopyable
    {
    public:
        PlanRows(const ISimpleIndex& index);

        // Constructs PlanRows from data previously persisted to a stream by
        // the Write() method.
        PlanRows(IInputStream& stream, const ISimpleIndex& index);

        virtual ~PlanRows();

        //
        // IPlanRows methods
        //

        ShardId GetShardCount() const;
        unsigned GetRowCount() const;

        const ITermTable& GetTermTable(ShardId shard) const;

        // Check if the number of rows in the PlanRows has
        // reached the limit as returned by GetRowCountLimit().
        bool IsFull() const;

        AbstractRow AddRow(Rank rank);

        RowId const & PhysicalRow(ShardId shard, unsigned id) const;
        RowId& PhysicalRow(ShardId shard, unsigned id);

        // Writes the PlanRows data to a stream.
        void Write(std::ostream& stream) const;

    protected:

        virtual unsigned GetRowCountLimit() const;

    private:
        class Entry
        {
        public:
            Entry(unsigned id, Rank rank);

            Entry(IInputStream& stream);

            unsigned GetId() const;
            Rank GetRank() const;

            RowId& operator[](ShardId shard);
            const RowId& operator[](ShardId shard) const;

            void Write(std::ostream& stream) const;

        private:
            unsigned m_id;
            Rank m_rank;
            RowId m_rowIds[c_maxShardIdCount];
        };

        const ISimpleIndex& m_index;

        FixedCapacityVector<Entry, c_maxRowsPerQuery> m_rows;
    };
}
