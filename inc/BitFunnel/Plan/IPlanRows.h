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
        virtual ~IPlanRows() {};

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
