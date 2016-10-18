#pragma once

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/IInterface.h"


namespace BitFunnel
{
    class IIngestionIndex;
    class IShardIndex;

    //*************************************************************************
    //
    // IIndexData is a base abstract class or interface for classes 
    // that provide maintain dynamic portion of the BitFunnel index. This data 
    // is usually a result of ingesting documents into the index and is used 
    // in query serving.
    //
    //*************************************************************************
    class IIndexData : public IInterface
    {
    public:
        // Returns the total document capacity of the BitFunnel index.
        virtual DocIndex GetTotalCapacity() const = 0;

        // Returns the shard index for a particular shard.
        virtual IShardIndex const & GetShardIndex(ShardId shardId) const = 0;

        // Returns an instance of the IIngestionIndex used for building the 
        // index.
        virtual IIngestionIndex& GetIngestionIndex() const = 0;
    };
}
