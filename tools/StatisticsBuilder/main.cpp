#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <vector>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/Stream.h"
#include "BitFunnel/TermInfo.h"
#include "CmdLineParser/CmdLineParser.h"
#include "DocumentDataSchema.h"
#include "DocumentHandleInternal.h"
#include "IndexUtils.h"
#include "Ingestor.h"
#include "IRecycler.h"
#include "MockTermTable.h"
#include "Recycler.h"
#include "SliceBufferAllocator.h"
// #include "TrackingSliceBufferAllocator.h"

// TODO: Fix the code below so that it actually compiles. It has dependencies
// on things in src/Index/src.
// Should we pull everything it depends on into inc?

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


    static void LoadAndIngestChunkList(char const * chunkListFileName)
    {
        // TODO: Add try/catch around file operations.
        std::cout << "Loading chunk list file '" << chunkListFileName << "'"
            << std::endl;
        std::vector<std::string> filePaths = ReadLines(chunkListFileName);

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

        const std::unique_ptr<IIngestor>
            ingestor(Factories::CreateIngestor(schema,
                                               *recycler,
                                               *termTable,
                                               *sliceAllocator));

        // Arbitrary maxGramSize that is greater than 1. For initial tests.
        // TODO: Choose correct maxGramSize.
        const size_t maxGramSize = 3;
        std::unique_ptr<IConfiguration>
            configuration(Factories::CreateConfiguration(maxGramSize));

        // TODO: Use correct thread count.
        size_t threadCount = 1;
        IngestChunks(filePaths, *configuration, *ingestor, threadCount);
        ingestor->PrintStatistics();

        ingestor->Shutdown();
        recycler->Shutdown();
    }
}


void CreateTestFiles(char const * chunkPath, char const * manifestPath)
{
    {
        std::ofstream chunkStream(chunkPath);

        char const chunk[] =

            // First document
            "Title\0Dogs\0\0"
            "Body\0Dogs\0are\0man's\0best\0friend.\0\0"
            "\0"

            // Second document
            "Title\0Cat\0Facts\0\0"
            "Body\0The\0internet\0is\0made\0of\0cats.\0\0"
            "\0"

            // Third document
            "Title\0More\0Cat\0Facts\0\0"
            "Body\0The\0internet\0really\0is\0made\0of\0cats.\0\0"
            "\0"

            // End of corpus
            "\0";

        // Write out sizeof(chunk) - 1 bytes to skip the trailing zero in corpus
        // which is not part of the file format.
        chunkStream.write(chunk, sizeof(chunk) - 1);
        chunkStream.close();
    }

    {
        std::ofstream manifestStream(manifestPath);
        manifestStream << chunkPath << std::endl;
        manifestStream.close();
    }
}


int main2(int /*argc*/, char** /*argv*/)
{
    CreateTestFiles(
        "/tmp/chunks/Chunk1",
        "/tmp/chunks/manifest.txt");
    return 0;
}


int main(int argc, char** argv)
{
    CmdLine::CmdLineParser parser(
        "StatisticsBuilder",
        "Ingest documents and compute statistics about them.");

    CmdLine::RequiredParameter<char const *> chunkListFileName(
        "chunkListFileName",
        "Path to a file containing the paths to the chunk files to be ingested. "
        "One chunk file per line. Paths are relative to working directory.");

    parser.AddParameter(chunkListFileName);

    int returnCode = 0;

    if (parser.TryParse(std::cout, argc, argv))
    {
        try
        {
            BitFunnel::LoadAndIngestChunkList(chunkListFileName);
            returnCode = 0;
        }
        catch (...)
        {
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
