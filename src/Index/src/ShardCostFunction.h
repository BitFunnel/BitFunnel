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

#include "BitFunnel/NonCopyable.h"              // Base class.
#include "BitFunnel/Index/IShardCostFunction.h" // Base class.


namespace BitFunnel
{
    class IDocumentHistogram;

    //*************************************************************************
    //
    // ShardCostFunction is an implementation of ICostFunction used in the
    // optimal shard construction algorithm. In optimal shard construction,
    // the shards are modeled as paths in a directed acyclic graph where
    // edges correspond to different posting counts.
    //
    //*************************************************************************
    class ShardCostFunction : public IShardCostFunction, NonCopyable
    {
    public:
        // Constructs a ShardCostFunction based on the supplied histogram.
        // A histogram with n entries maps to a graph with n + 1 vertices.
        // The shardOverhead parameter provides the weight for combining the
        // runtime overhead of a shard with its memory cost. The minShardCapacity
        // parameter specifies the minimal allowed capacity of any shard.
        ShardCostFunction(IDocumentHistogram const & histogram,
                          double shardOverhead,
                          size_t minShardCapacity,
                          Rank maxRankInUse);

        // Configures the cost function for the empty shard starting and ending
        // immediately before the specified histogram slot. Initializes
        // accumulators used by the Extend() and GetCost() methods.
        void StartAt(size_t vertex);

        // Extends the shard to cover the next histogram slot and updates
        // accumulators accordingly.
        void Extend();

        // Returns the cost metric for the shard specified by StartAt() and
        // Extend().
        float GetCost() const;

        // Returns the number of vertices in the graph which is equal to one
        // more than the number of slots in the histogram.
        size_t GetVertexCount() const;

        // Adds the current shard to the specified IShardDefinition.
        void AddShard(IShardDefinition& shardDefinition) const;

    private:
        //
        // Constructor paramters
        //

        const IDocumentHistogram& m_histogram;
        const double m_shardOverhead;
        const size_t m_minShardCapacity;
        const Rank m_maxRankInUse;

        //
        // Other members.
        //

        // Total number of documents in histogram slots starting at
        // m_fromVertex and continuing to through  m_toVertex.
        double m_documentCount;

        // Total number of postings in above set of histogram slots.
        double m_postingCountSum;

        // Number of postings in histogram slot m_toVertex.
        size_t m_maxPostingCount;

        // First histogram slot in shard being costed.
        size_t m_fromVertex;

        // Last histogram slot in shard being costed.
        size_t m_toVertex;
    };
}
