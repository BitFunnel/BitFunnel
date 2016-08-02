#include <algorithm>
#include <future>
#include <fstream>
#include <iostream>
#include <memory>
#include <stddef.h>
#include <string>
#include <vector>

#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/Stream.h"
#include "CmdLineParser/CmdLineParser.h"
#include "DocumentDataSchema.h"
#include "IndexUtils.h"
#include "IRecycler.h"
#include "MockTermTable.h"
#include "Recycler.h"
#include "SliceBufferAllocator.h"
// #include "TrackingSliceBufferAllocator.h"

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


    static void LoadAndIngestChunkList(char const * chunkListFileName,
                                       char const * docFreqTableFileName,
                                       char const * docLengthHistogramFileName,
                                       char const * cumulativePostingCountsFileName)
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
        const size_t maxGramSize = 1;
        std::unique_ptr<IConfiguration>
            configuration(Factories::CreateConfiguration(maxGramSize));

        std::cout << "Ingesting . . ." << std::endl;

        // TODO: Use correct thread count.
        size_t threadCount = 1;
        IngestChunks(filePaths, *configuration, *ingestor, threadCount);

        std::cout << "Ingestion complete." << std::endl;
        ingestor->PrintStatistics();

        if (docFreqTableFileName != nullptr)
        {
            std::cout
                << "Writing document frequency table to "
                << docFreqTableFileName
                << std::endl;

            std::ofstream out(docFreqTableFileName);
            ingestor->WriteDocumentFrequencyTable(out);
        }

        if (docLengthHistogramFileName != nullptr)
        {
            std::cout
                << "Writing document length histogram to "
                << docLengthHistogramFileName
                << std::endl;

            std::ofstream out(docLengthHistogramFileName);
            ingestor->WriteDocumentLengthHistogram(out);
        }

        if (cumulativePostingCountsFileName != nullptr)
        {
            std::cout
                << "Writing cumulative posting counts "
                << cumulativePostingCountsFileName
                << std::endl;

            std::ofstream out(cumulativePostingCountsFileName);
            ingestor->WriteCumulativePostingCounts(out);
        }

        ingestor->Shutdown();
        recycler->Shutdown();
        background.wait();
    }
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

    CmdLine::OptionalParameter<char const *> docFrequencyTableFileName(
        "docFreqTable",
        "Path to file where document frequency table will be written.",
        nullptr);

    CmdLine::OptionalParameter<char const *> docLengthHistogramFileName(
        "docLengthHistogram",
        "Path to file where document length histogram will be written.",
        nullptr);

    CmdLine::OptionalParameter<char const *> cumulativePostingCountsFilename(
        "cumulativePostingCountss",
        "Path to file where the table of cumulative posting counts will be written.",
        nullptr);

    parser.AddParameter(chunkListFileName);
    parser.AddParameter(docFrequencyTableFileName);
    parser.AddParameter(docLengthHistogramFileName);
    parser.AddParameter(cumulativePostingCountsFilename);

    int returnCode = 0;

    if (parser.TryParse(std::cout, argc, argv))
    {
        try
        {
            if (docFrequencyTableFileName.HasValue())
            {
                std::cout << "docFrequencyTableFileName: " << docFrequencyTableFileName << std::endl;
            }

            if (docLengthHistogramFileName.HasValue())
            {
                std::cout << "docLengthHistogramFileName: " << docLengthHistogramFileName << std::endl;
            }

            if (cumulativePostingCountsFilename.HasValue())
            {
                std::cout << "cumulativePostingCountsFilename: " << cumulativePostingCountsFilename << std::endl;
            }

            BitFunnel::LoadAndIngestChunkList(chunkListFileName,
                                              docFrequencyTableFileName,
                                              docLengthHistogramFileName,
                                              cumulativePostingCountsFilename);
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
