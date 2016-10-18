#pragma once

// TODO: should this be in Index?

#include <memory>                         // Uses ptrdiff_t.

#include "BitFunnel/BitFunnelTypes.h"     // Row and ShardId parameters.
#include "BitFunnel/Row.h"


namespace BitFunnel
{
    class Context;
    class IRowsAvailable;


    //*************************************************************************
    //
    // IRowSet is an abstract base class or interface for objects that provide
    // arrays of row offsets for each Shard.
    //
    //*************************************************************************
    class IRowSet
    {
    public:
        virtual ~IRowSet() {};

        // Instructs the IRowSet to load into memory those rows that reside on
        // secondary storage such as SSD and HDD. Some implementations of the
        // may be asynchronous. A callback to the OnRowsAvailable() method on
        // the rowsAvailable parameter indicates that the loads have completed.
        // This callback my be synchronous (i.e. on the current thread before
        // the LoadRows() method returns) or asynchronous (i.e. on a different
        // thread at any point in the future).
        // This function is intended to be called in sequential manner. When called
        // the first time, it will load the rows. For all subsequent calls, no rows
        // will be loaded again. However, The IRowAvailable callback function will
        // be called on either cases.
        virtual void LoadRows(Context const & context,
                              IRowsAvailable& rowsAvailable) = 0;

        // Returns the number of Shards in the RowSet.
        virtual ShardId GetShardCount() const = 0;

        // Returns the number of rows in each Shard. Note that the RowSet will
        // contain a total of GetShardCount() * GetRowCount() rows.
        virtual unsigned GetRowCount() const = 0;

        // Returns the array of offsets for rows associated with a specified
        // shard. The offset specified where in the slice buffer a particular
        // row is stored.
        virtual ptrdiff_t const * GetRowOffsets(ShardId shard) const = 0;
    };
}
