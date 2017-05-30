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

#include <iosfwd>                       // std::ostream used as a parameter.

#include "BitFunnel/BitFunnelTypes.h"   // ShardId used as a parameter.
#include "BitFunnel/IInterface.h"       // Base class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // Abstract base class for objects that describe a collection of index 
    // shards. Each shard specifies the minimum number of postings allowed
    // for documents in the shard. For shards other than the last shard, the
    // maximum number of postings per document is one less than the minimum
    // allowed in the next shard. The last shard allows all documents with a
    // posting count at least as large as its minimum posting count.
    //
    //*************************************************************************
    class IShardDefinition : public IInterface
    {
    public:
        // Persists the ShardDefinition data to a stream. Typically classes
        // that implement IShardDefinition also implement a constructor that
        // reconstitutes the class from a stream.
        virtual void Write(std::ostream& output) const = 0;

        // Adds a shard to the collection. The minPostingCount parameter
        // specifies the minimum number of postings for any document that is
        // considered a member of the shard. The density parameter specifies
        // the target density for rows in the shard. This must be the same
        // density specified when building the TermTable for this shard.
        virtual void AddShard(size_t minPostingCount, double density) = 0;

        // Returns the ShardId that contains documents with the specified
        // postingCount.
        virtual ShardId GetShard(size_t postingCount) const = 0;

        // Returns the minimum posting count for documents in the specified
        // shard.
        virtual size_t GetMinPostingCount(ShardId shard) const = 0;

        // Returns the maximum posting count for documents in the specified
        // shard.
        virtual size_t GetMaxPostingCount(ShardId shard) const = 0;

        // Returns the bit densitry for the specified shard.
        virtual double GetDensity(ShardId shard) const = 0;

        // Returns the number of shards in the map.
        virtual ShardId GetShardCount() const = 0;
    };
}
