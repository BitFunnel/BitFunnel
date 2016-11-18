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


#include <iostream>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocumentHistogram.h"
#include "BitFunnel/Index/IShardCostFunction.h"
#include "BitFunnel/Index/ShardDefinitionBuilder.h"
#include "BitFunnel/IFileManager.h"
#include "CmdLineParser/CmdLineParser.h"
#include "ShardBuilder.h"


namespace BitFunnel
{
    ShardBuilder::ShardBuilder(IFileSystem& fileSystem)
        : m_fileSystem(fileSystem)
    {
    }


    int ShardBuilder::Main(std::istream& /*input*/,
                                   std::ostream& output,
                                   int argc,
                                   char const *argv[])
    {
        CmdLine::CmdLineParser parser(
            "ShardBuilder Tool",
            "Creates an optimal ShardDefinition from a DocumentHistogram.");

        CmdLine::RequiredParameter<char const *> path(
            "path",
            "Path to directory with DocumentHistogram.csv file.");

        CmdLine::OptionalParameter<int> maxShardCount(
            "maxshards",
            "Set the maximum number of shards.",
            10u,
            CmdLine::GreaterThan(0));

        CmdLine::OptionalParameter<double> overhead(
            "overhead",
            "the weight for combining the runtime overhead of a shard with its memory cost",
            1.0);

        CmdLine::OptionalParameter<int> minCapacity(
            "mincapacity",
            "Set the minimum shard capacity.",
            3u,
            CmdLine::GreaterThan(0));

        CmdLine::OptionalParameter<int> maxRankInUse(
            "maxrank",
            "Set the maximum rank in use.",
            3u,
            CmdLine::GreaterThan(0));

        parser.AddParameter(path);
        parser.AddParameter(maxShardCount);
        parser.AddParameter(overhead);
        parser.AddParameter(minCapacity);
        parser.AddParameter(maxRankInUse);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                Go(path,
                   static_cast<size_t>(maxShardCount),
                   overhead,
                   static_cast<size_t>(minCapacity),
                   static_cast<size_t>(maxRankInUse));

                returnCode = 0;
            }
            catch (RecoverableError e)
            {
                output << "Error: " << e.what() << std::endl;
            }
            catch (...)
            {
                output << "Unexpected error.";
            }
        }

        return returnCode;
    }


    void ShardBuilder::Go(char const * path,
                          size_t maxShardCount,
                          double shardOverhead,
                          size_t minShardCapacity,
                          Rank maxRankInUse) const
    {
        //size_t maxShardCount = 10;
        //double shardOverhead = 1.0;
        //size_t minShardCapacity = 1;
        //Rank maxRankInUse = 3;

        std::cout
            << "Optimal Shard Builder" << std::endl
            << "  maxShards: " << maxShardCount << std::endl
            << "  shardOverhead: " << shardOverhead << std::endl
            << "  minShardCapacity: " << minShardCapacity << std::endl
            << "  maxRankInUse: " << maxRankInUse << std::endl
            << std::endl;

        auto fileManager = Factories::CreateFileManager(path,
                                                        path,
                                                        path,
                                                        m_fileSystem);

        std::cout
            << "Reading from "
            << fileManager->DocumentHistogram().GetName()
            << std::endl
            << std::endl;

        auto histogram = Factories::CreateDocumentHistogram(
            *fileManager->DocumentHistogram().OpenForRead());

        std::cout
            << "Shuffling. Please wait ..."
            << std::endl
            << std::endl;

        auto costFunction =
            Factories::CreateShardCostFunction(*histogram,
                                               1.0,
                                               1,
                                               3);

        auto shardDefinition =
            ShardDefinitionBuilder::CreateShardDefinition(*costFunction,
                                                          maxShardCount);

        std::cout
            << "Writing to "
            << fileManager->ShardDefinition().GetName()
            << std::endl
            << std::endl;

        auto output = fileManager->ShardDefinition().OpenForWrite();
        shardDefinition->Write(*output);
    }
}
