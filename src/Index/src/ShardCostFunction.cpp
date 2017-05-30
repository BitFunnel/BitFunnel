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

#include <algorithm>    // std::min() or std::max()

#include "BitFunnel/Configuration/IShardDefinition.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocumentHistogram.h"
#include "BitFunnel/Index/Row.h"
#include "LoggerInterfaces/Logging.h"
#include "ShardCostFunction.h"


namespace BitFunnel
{
    std::unique_ptr<IShardCostFunction>
        Factories::CreateShardCostFunction(
            IDocumentHistogram const & histogram,
            double shardOverhead,
            size_t minShardCapacity,
            Rank maxRankInUse)
    {
        return std::unique_ptr<IShardCostFunction>(
            new ShardCostFunction(histogram,
                                  shardOverhead,
                                  minShardCapacity,
                                  maxRankInUse));
    }

    //*************************************************************************
    //
    // ShardCostFunction
    //
    //*************************************************************************
    ShardCostFunction::ShardCostFunction(
        IDocumentHistogram const & histogram,
        double shardOverhead,
        size_t minShardCapacity,
        Rank maxRankInUse)
        : m_histogram(histogram),
          m_shardOverhead(shardOverhead),
          m_minShardCapacity(minShardCapacity),
          m_maxRankInUse(maxRankInUse)
    {
        LogAssertB(m_histogram.GetEntryCount() > 0, "");

        StartAt(0);
    }


    void ShardCostFunction::StartAt(size_t vertex)
    {
        m_fromVertex = vertex;
        m_toVertex = vertex;
        m_documentCount = 0;
        m_postingCountSum = 0;
        m_maxPostingCount = 0;
    }


    void ShardCostFunction::Extend()
    {
        double documentCount = m_histogram.GetDocumentCount(m_toVertex);
        m_documentCount += documentCount;

        size_t postingCount = m_histogram.GetPostingCount(m_toVertex);
        m_postingCountSum += postingCount * documentCount;
        m_maxPostingCount = (std::max)(postingCount, m_maxPostingCount);

        m_toVertex++;
    }


    float ShardCostFunction::GetCost() const
    {
        // The number of columns in a row table is formed by rounding up the
        // number of documents to meet certain alignment constraints.
        // TODO: REVIEW: Is this well behaved when m_documentCount == 0?
        // This is a valid case during the optimzation, even though the optimal
        // solution will never contain a shard with zero documents.
        double columnCost = static_cast<double>(
            Row::DocumentsInRank0Row(static_cast<DocIndex>(m_documentCount),
                                     m_maxRankInUse));

        if (columnCost < m_minShardCapacity)
        {
            // Don't allow a shard which has a capacity less than m_minShardCapacity.
            return std::numeric_limits<float>::infinity();
        }

        // The number of rows in a row table is proportional to the number of
        // postings in the target document size for this shard. This cost
        // function assumes that the target document size will be that of the
        // document in the shard with the most postings.
        // TODO: Might want to consider using the mean or median posting count
        // for the target.
        double rowCost = static_cast<double>(m_maxPostingCount);

        // The memory used by row tables is proportional to the product of the
        // number of columns and number of rows.
        double memoryCost = columnCost * rowCost;

        // Each shard has a runtime cost associated with the fixed overhead of
        // planning and accessing rows from SSD during matcher execution. This
        // cost is weighted by an empirically determined factor to capture the
        // relative cost of memory vs execution speed.

        double totalCost = memoryCost + m_shardOverhead;

        return static_cast<float>(totalCost);
    }


    size_t ShardCostFunction::GetVertexCount() const
    {
        return static_cast<unsigned>(m_histogram.GetEntryCount() + 1);
    }


    void ShardCostFunction::AddShard(IShardDefinition& shardDefinition) const
    {
        // TODO: Issue #396. Is there some way to provider a density
        // other than the default?
        const double defaultDensity = 0.15;
        shardDefinition.AddShard(m_maxPostingCount, defaultDensity);
    }
}
