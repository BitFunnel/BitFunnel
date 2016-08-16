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

#include <algorithm>
#include <fstream>
#include <future>
#include <fstream>
#include <iostream>
#include <memory>
#include <stddef.h>
#include <string>
#include <vector>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IShardDefinition.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIndexedIdfTable.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/Stream.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "CmdLineParser/CmdLineParser.h"
#include "DocumentDataSchema.h"
#include "IndexUtils.h"
#include "IRecycler.h"
#include "MockTermTable.h"
#include "Recycler.h"
#include "SliceBufferAllocator.h"
// #include "TrackingSliceBufferAllocator.h"

#include "Headers.h"


namespace BitFunnel
{
    // Returns a vector with one entry for each line in the file.
    static std::vector<std::string> ReadLines(char const * fileName)
    {
        std::ifstream file(fileName);

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(std::move(line));
        }

        return lines;
    }


    void AddTerm(MockTermTable& termTable, char const * termText)
    {
        const Term term= Term(Term::ComputeRawHash(termText), StreamId::Full, 0);
        // TODO: 0 is arbitrary.
        termTable.AddTerm(term.GetRawHash(), 0, 1);
    }


    static void LoadAndIngestChunkList(char const * intermediateDirectory,
                                       char const * chunkListFileName,
                                       // TODO: gramSize should be unsigned once CmdLineParser supports unsigned.
                                       int gramSize,
                                       bool generateStatistics,
                                       bool generateTermToText)
    {
        if (gramSize < 0 || gramSize > Term::c_maxGramSize)
        {
            throw FatalError("ngram size out of range.");
        }

        auto fileManager = Factories::CreateFileManager(intermediateDirectory,
                                                        intermediateDirectory,
                                                        intermediateDirectory);

        // TODO: Add try/catch around file operations.
        std::cout << "Loading chunk list file '" << chunkListFileName << "'"
            << std::endl;
        std::cout << "Temp dir: '" << intermediateDirectory << "'"
            << std::endl;
        std::vector<std::string> filePaths = ReadLines(chunkListFileName);

        std::cout << "Reading " << filePaths.size() << " files\n";

        DocumentDataSchema schema;

        std::unique_ptr<IRecycler> recycler =
            std::unique_ptr<IRecycler>(new Recycler());
        auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

        static const std::vector<RowIndex>
            // 4 rows for private terms, 1 row for a fact.
            rowCounts = { c_systemRowCount + 4 + 1, 0, 0, 0, 0, 0, 0 };
        std::shared_ptr<ITermTable const> termTable(new MockTermTable(0));
        MockTermTable& mockTermTable = const_cast<MockTermTable&>(
            dynamic_cast<MockTermTable const &>(*termTable));

        AddTerm(mockTermTable, "this");
        AddTerm(mockTermTable, "is");
        AddTerm(mockTermTable, "a");
        AddTerm(mockTermTable, "test");

        static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);
        const size_t sliceBufferSize = GetBufferSize(c_sliceCapacity, schema, *termTable);

        std::unique_ptr<SliceBufferAllocator>
            sliceAllocator(new SliceBufferAllocator(sliceBufferSize, 16));

        auto shardDefinition = Factories::CreateShardDefinition();
        // shardDefinition->AddShard(1000);
        // shardDefinition->AddShard(2000);
        // shardDefinition->AddShard(3000);

        const std::unique_ptr<IIngestor>
            ingestor(Factories::CreateIngestor(*fileManager,
                                               schema,
                                               *recycler,
                                               *termTable,
                                               *shardDefinition,
                                               *sliceAllocator));

        const std::unique_ptr<IIndexedIdfTable>
            idfTable(Factories::CreateIndexedIdfTable());


        // Arbitrary maxGramSize that is greater than 1. For initial tests.
        // TODO: Choose correct maxGramSize.
        std::unique_ptr<IConfiguration>
            configuration(
                Factories::CreateConfiguration(
                    static_cast<Term::GramSize>(gramSize),
                    generateTermToText,
                    *idfTable));

        std::cout << "Ingesting . . ." << std::endl;

        Stopwatch stopwatch;

        // TODO: Use correct thread count.
        const size_t threadCount = 1;
        IngestChunks(filePaths, *configuration, *ingestor, threadCount);

        const double elapsedTime = stopwatch.ElapsedTime();
        const size_t totalSourceBytes = ingestor->GetTotalSouceBytesIngested();

        std::cout << "Ingestion complete." << std::endl;
        std::cout << "  Ingestion time = " << elapsedTime << std::endl;
        std::cout << "  Ingestion rate (bytes/s): " << totalSourceBytes / elapsedTime << std::endl;

        ingestor->PrintStatistics();

        if (generateStatistics)
        {
            TermToText const * termToText = nullptr;
            if (configuration->KeepTermText())
            {
                termToText = &configuration->GetTermToText();
            }
            ingestor->WriteStatistics(termToText);
        }

        ingestor->Shutdown();
        recycler->Shutdown();
        background.wait();
    }
}


int main2(int argc, char** argv)
{
    CmdLine::CmdLineParser parser(
        "StatisticsBuilder",
        "Ingest documents and compute statistics about them.");

    CmdLine::RequiredParameter<char const *> chunkListFileName(
        "chunkListFileName",
        "Path to a file containing the paths to the chunk files to be ingested. "
        "One chunk file per line. Paths are relative to working directory.");

    CmdLine::RequiredParameter<char const *> tempPath(
        "tempPath",
        "Path to a tmp directory. "
        "Something like /tmp/ or c:\\temp\\, depending on platform..");

    CmdLine::OptionalParameterList statistics(
        "statistics",
        "Generate index statistics such as document frequency table, "
        "document length histogram, and cumulative term counts.");

    CmdLine::OptionalParameterList termToText(
        "text",
        "Create mapping from Term::Hash to term text.");

    // TODO: This parameter should be unsigned, but it doesn't seem to work
    // with CmdLineParser.
    CmdLine::OptionalParameter<int> gramSize(
        "gramsize",
        "Set the maximum ngram size for phrases.",
        1u);

    parser.AddParameter(chunkListFileName);
    parser.AddParameter(tempPath);
    parser.AddParameter(statistics);
    parser.AddParameter(termToText);
    parser.AddParameter(gramSize);

    int returnCode = 0;

    if (parser.TryParse(std::cout, argc, argv))
    {
        try
        {
            BitFunnel::LoadAndIngestChunkList(tempPath,
                                              chunkListFileName,
                                              gramSize,
                                              statistics.IsActivated(),
                                              termToText.IsActivated());
            returnCode = 0;
        }
        catch (...)
        {
            // TODO: Do we really want to catch all exceptions here?
            // Seems we want to at least print out the error message for BitFunnel exceptions.

            std::cout << "Unexpected error.";
            returnCode = 1;
        }
    }
    else
    {
        parser.Usage(std::cout, argv[0]);
        returnCode = 1;
    }

    return returnCode;
}


namespace BitFunnel
{
    void Test()
    {
        TaskFactory factory;

        DelayedPrint::Register(factory);
        Exit::Register(factory);

        for (;;)
        {
            try
            {
                std::cout << factory.GetNextTaskId() << ": ";
                std::cout.flush();

                std::string line;
                std::getline(std::cin, line);

                std::unique_ptr<ITask> task(factory.CreateTask(line.c_str()));

                if (task->GetType() == ITask::Type::Exit)
                {
                    break;
                }

                task->Execute();
            }
            catch (RecoverableError e)
            {
                std::cout
                    << "Error: "
                    << e.what()
                    << std::endl;
            }
        }

        //std::unique_ptr<ITask> task(factory.CreateTask("delay hello"));

        //task->Execute();

//        auto v = factory.Tokenize("one two \"three four\" five");
    }
}


int main(int /*argc*/, char** /*argv*/)
{
    BitFunnel::Test();
}
