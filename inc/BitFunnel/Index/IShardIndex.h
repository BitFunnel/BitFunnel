#pragma once

#include <memory>                         // For ptrdiff_t.
#include <vector>                         // For function return value.

#include "BitFunnel/BitFunnelTypes.h"     // ShardId and DocIndex parameter.
#include "BitFunnel/IInterface.h"
#include "BitFunnel/Index/RowId.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // IShardIndex is a base abstract class or interface for classes that
    // maintain dynamic portion of the BitFunnel index for a particular shard.
    // This data is used in query serving.
    //
    //*************************************************************************
    class IShardIndex : public IInterface
    {
    public:
        // Returns the Id of the shard.
        virtual ShardId GetId() const = 0;

        // Returns capacity of a single Slice in the Shard. All Slices in the
        // Shard have the same capacity.
        virtual DocIndex GetSliceCapacity() const = 0;

        // Returns a vector of slice buffers for this shard.
        // The callers needs to obtain a Token from ITokenManager to protect
        // the pointer to the list of slice buffers, as well as the buffers
        // themselves.
        virtual std::vector<void*> const & GetSliceBuffers() const = 0;

        // Returns the offset of the row in the slice buffer in a shard.
        virtual ptrdiff_t GetRowOffset(RowId rowId) const = 0;

        // Returns the offset in the slice buffer where a pointer to the Slice
        // is stored. This is the same offset for all slices in the Shard.
        virtual ptrdiff_t GetSlicePtrOffset() const = 0;
    };
}
