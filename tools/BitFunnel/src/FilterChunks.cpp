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

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "BitFunnel/Chunks/DocumentFilters.h"
#include "BitFunnel/Chunks/Factories.h"
#include "BitFunnel/Chunks/IChunkManifestIngestor.h"
#include "BitFunnel/Chunks/IChunkProcessor.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "CmdLineParser/CmdLineParser.h"
#include "FilterChunks.h"


namespace BitFunnel
{
    FilterChunks::FilterChunks(IFileSystem& fileSystem)
        : m_fileSystem(fileSystem)
    {
    }


    int FilterChunks::Main(std::istream& /*input*/,
                           std::ostream& output,
                           int argc,
                           char const *argv[])
    {
        CmdLine::CmdLineParser parser(
            "FilterChunks",
            "Filter chunk files by posting count, then take random sample.");

        CmdLine::RequiredParameter<char const *> manifestFileName(
            "manifestFile",
            "Path to a file containing the paths to the chunk files to be ingested. "
            "One chunk file per line. Paths are relative to working directory.");

        CmdLine::RequiredParameter<char const *> outputPath(
            "outDir",
            "Path to the output directory where files will be written. ");

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> gramSize(
            "gramsize",
            "Set the maximum ngram size for phrases.",
            1u);

        CmdLine::OptionalParameterList random(
            "random",
            "Sample a random fraction of the corpus.");
        CmdLine::RequiredParameter<int> seed(
            "seed",
            "random number generator seed.");
        CmdLine::RequiredParameter<double> fraction(
            "fraction",
            "fraction of corpus to sample.");
        random.AddParameter(seed);
        random.AddParameter(fraction);

        CmdLine::OptionalParameterList size(
            "size",
            "Sample documents by posting count.");
        CmdLine::RequiredParameter<int> minCount(
            "min postings",
            "minimum number of postings.");
        CmdLine::RequiredParameter<int> maxCount(
            "max postings",
            "maximum number of postings.");
        size.AddParameter(minCount);
        size.AddParameter(maxCount);

        CmdLine::OptionalParameter<int> count(
            "count",
            "Maximum number of documents.",
            1u);


        parser.AddParameter(manifestFileName);
        parser.AddParameter(outputPath);
        parser.AddParameter(gramSize);
        parser.AddParameter(random);
        parser.AddParameter(size);
        parser.AddParameter(count);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                CompositeFilter filter;

                if (size.IsActivated())
                {
                    filter.AddFilter(
                        std::unique_ptr<IDocumentFilter>(
                            new PostingCountFilter(
                                static_cast<unsigned>(minCount),
                                static_cast<unsigned>(maxCount))));
                }

                if (count.IsActivated())
                {
                    filter.AddFilter(
                        std::unique_ptr<IDocumentFilter>(
                            new DocumentCountFilter(static_cast<unsigned>(count))));
                }

                if (random.IsActivated())
                {
                    filter.AddFilter(
                        std::unique_ptr<IDocumentFilter>(
                            new RandomDocumentFilter(static_cast<unsigned>(seed),
                                                     fraction)));
                }

                FilterChunkList(output,
                                outputPath,
                                manifestFileName,
                                gramSize);
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


    void FilterChunks::FilterChunkList(
        std::ostream& output,
        char const * intermediateDirectory,
        char const * chunkListFileName,
        // TODO: gramSize should be unsigned once CmdLineParser supports unsigned.
        int gramSize) const
    {
        // TODO: cast of gramSize can be removed when it's fixed to be unsigned.
        auto index = Factories::CreateSimpleIndex(m_fileSystem);
        index->ConfigureForStatistics(intermediateDirectory,
                                      static_cast<size_t>(gramSize),
                                      false);
        index->StartIndex();


        // TODO: Add try/catch around file operations.
        output
            << "Loading chunk list file '" << chunkListFileName << "'" << std::endl
            << "Temp dir: '" << intermediateDirectory << "'" << std::endl;

        std::vector<std::string> filePaths = ReadLines(m_fileSystem, chunkListFileName);

        output << "Reading " << filePaths.size() << " files\n";

        IConfiguration const & configuration = index->GetConfiguration();
        IIngestor & ingestor = index->GetIngestor();

        NopFilter filter;

        auto manifest = Factories::CreateChunkManifestIngestor(
            m_fileSystem,
            nullptr,
            filePaths,
            configuration,
            ingestor,
            filter,
            false);

        output << "Filtering chunks . . ." << std::endl;

        Stopwatch stopwatch;

        for (size_t i = 0; i < filePaths.size(); ++i)
        {
            manifest->IngestChunk(i);
        }

        const double elapsedTime = stopwatch.ElapsedTime();
        const size_t totalSourceBytes = ingestor.GetTotalSouceBytesIngested();

        output
            << "Filtering complete." << std::endl
            << "  Ingestion time = " << elapsedTime << std::endl
            << "  Ingestion rate (bytes/s): "
            << totalSourceBytes / elapsedTime << std::endl;
    }
}
