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

#include <memory>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/IShardCostFunction.h"
#include "BitFunnel/Index/ShardDefinitionBuilder.h"
#include "SingleSourceShortestPath.h"


namespace BitFunnel
{
    //*********************************************************************
    //
    // CreateShardDefinition() constructs an optimal shard definition based
    // on corpus data in an IDocumentHistogram and an ICostFunction which
    // assigns costs to proposed shards.
    //
    // CreateShardDefinition() uses an algorithm that is guaranteed to find
    // the set of shards which minimize the sum of shard costs. The
    // algorithm is combinatorial and places no restrictions on the
    // mathematical properties of the cost function.
    //
    //*********************************************************************
    std::unique_ptr<IShardDefinition const>
    ShardDefinitionBuilder::CreateShardDefinition(IShardCostFunction& costFunction,
                                                  size_t maxShardCount)
    {
        //
        // Use the Single Source Shortest Path algorithm to find the path that
        // corresponds to the lowest cost set of shards.
        //
        std::vector<size_t> path;
        SingleSourceShortestPath::FindPath(costFunction, maxShardCount, path);

        //
        // Convert the path into a ShardDefinition.
        //
        auto shardDefinition =
            Factories::CreateShardDefinition();

        size_t index = 0;
        costFunction.StartAt(index);
        for (unsigned i = 1 ; i < path.size(); ++i)
        {
            while (index < path[i])
            {
                ++index;
                costFunction.Extend();
            }
            costFunction.AddShard(*shardDefinition);
            costFunction.StartAt(index);
        }

        return std::move(shardDefinition);
    }
}
