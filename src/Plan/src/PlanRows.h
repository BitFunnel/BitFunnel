#pragma once

#include <iosfwd>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/Plan/IPlanRows.h"
#include "FixedCapacityVector.h"


namespace BitFunnel
{
    class IIndexConfiguration;
    class IInputStream;


    class PlanRows : public IPlanRows, NonCopyable
    {
    public:
        PlanRows(const IIndexConfiguration& index);

        // Constructs PlanRows from data previously persisted to a stream by
        // the Write() method.
        PlanRows(IInputStream& stream, const IIndexConfiguration& index);

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

        const IIndexConfiguration& m_index;

        FixedCapacityVector<Entry, c_maxRowsPerQuery> m_rows;
    };
}
