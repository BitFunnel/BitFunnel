#include "stdafx.h"

#include <memory>

#include "BitFunnel/Factories.h"
#include "BitFunnel/IDocumentHistogram.h"
#include "BitFunnel/IShardCostFunction.h"
#include "BitFunnel/IShardDefinition.h"
#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/ShardDefinitionBuilder.h"
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
    const IShardDefinition*
    ShardDefinitionBuilder::CreateShardDefinition(IShardCostFunction& costFunction, unsigned maxShardCount)
    {
        //
        // Use the Single Source Shortest Path algorithm to find the path that
        // corresponds to the lowest cost set of shards.
        //
        std::vector<unsigned> path;
        SingleSourceShortestPath::FindPath(costFunction, maxShardCount, path);

        //
        // Convert the path into a ShardDefinition.
        //
        std::auto_ptr<IShardDefinition> shardDefinition(Factories::CreateShardDefinition());
        unsigned index = 0;
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

        return shardDefinition.release();
    }
}
