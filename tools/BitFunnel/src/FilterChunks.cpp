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
#include <vector>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Chunks/DocumentFilters.h"
#include "BitFunnel/Chunks/Factories.h"
#include "BitFunnel/Chunks/IChunkManifestIngestor.h"
#include "BitFunnel/Chunks/IChunkWriter.h"
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
#include "LoggerInterfaces/Check.h"


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
            "Copies a set of chunk files specified by a manifest "
            "while filtering their documents based on a set of predicates.");

        CmdLine::RequiredParameter<char const *> manifestFileName(
            "manifestFile",
            "Path to a file containing the paths to the chunk files to be copied. "
            "One chunk file per line. Paths are relative to working directory.");

        CmdLine::RequiredParameter<char const *> outputPath(
            "outDir",
            "Path to the output directory where the filtered "
            "chunk files will be written.");

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> gramSize(
            "gramsize",
            "Set the maximum ngram size for phrases.",
            1u,
            CmdLine::GreaterThan(0));

        CmdLine::OptionalParameterList random(
            "random",
            "Copy a random fraction of the corpus.");
        CmdLine::RequiredParameter<int> seed(
            "seed",
            "random number generator seed.");
        CmdLine::RequiredParameter<double> fraction(
            "fraction",
            "fraction of corpus to copy.",
            CmdLine::Range(CmdLine::GreaterThanOrEqual(0.0),
                           CmdLine::LessThanOrEqual(1.0)));
        random.AddParameter(seed);
        random.AddParameter(fraction);

        CmdLine::OptionalParameterList size(
            "size",
            "Sample documents by posting count.");
        CmdLine::RequiredParameter<int> minCount(
            "min postings",
            "minimum number of postings (inclusive).",
            CmdLine::GreaterThanOrEqual(0));
        CmdLine::RequiredParameter<int> maxCount(
            "max postings",
            "maximum number of postings (inclusive).");
        size.AddParameter(minCount);
        size.AddParameter(maxCount);

        CmdLine::OptionalParameter<int> count(
            "count",
            "Maximum number of documents.",
            1u,
            CmdLine::GreaterThan(0));

        CmdLine::OptionalParameter<const char *> writer(
            "writer",
            "Specify chunk writer (annotate or copy)",
            "copy");


        parser.AddParameter(manifestFileName);
        parser.AddParameter(outputPath);
        parser.AddParameter(gramSize);
        parser.AddParameter(random);
        parser.AddParameter(size);
        parser.AddParameter(count);
        parser.AddParameter(writer);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                CompositeFilter filter;

                if (size.IsActivated())
                {
                    CHECK_LE(minCount, maxCount)
                        << "Invalid posting count range.";
                    filter.AddFilter(
                        std::unique_ptr<IDocumentFilter>(
                            new PostingCountFilter(
                                static_cast<unsigned>(minCount),
                                static_cast<unsigned>(maxCount))));
                }

                if (random.IsActivated())
                {
                    filter.AddFilter(
                        std::unique_ptr<IDocumentFilter>(
                            new RandomDocumentFilter(fraction,
                                                     static_cast<unsigned>(seed))));
                }

                if (count.IsActivated())
                {
                    filter.AddFilter(
                        std::unique_ptr<IDocumentFilter>(
                            new DocumentCountFilter(static_cast<unsigned>(count))));
                }

                FilterChunkList(output,
                                outputPath,
                                manifestFileName,
                                gramSize,
                                filter,
                                writer);

                returnCode = 0;
            }
            catch (std::exception e)
            {
                output << "Error: " << e.what() << std::endl;
            }
            catch (Logging::CheckException e)
            {
                output << "Error: " << e.GetMessage().c_str() << std::endl;
            }
            catch (...)
            {
                output << "Unexpected error." << std::endl;
            }
        }

        return returnCode;
    }


    void FilterChunks::FilterChunkList(
        std::ostream& output,
        char const * outputDirectory,
        char const * chunkListFileName,
        // TODO: gramSize should be unsigned once CmdLineParser supports unsigned.
        int gramSize,
        IDocumentFilter & filter,
        char const * writer) const
    {
        // TODO: cast of gramSize can be removed when it's fixed to be unsigned.
        auto index = Factories::CreateSimpleIndex(m_fileSystem);
        index->ConfigureForStatistics(outputDirectory,
                                      static_cast<size_t>(gramSize),
                                      false);
        index->StartIndex();


        // TODO: Add try/catch around file operations.
        output
            << "Loading chunk list file '" << chunkListFileName << "'" << std::endl
            << "Output directory: '" << outputDirectory << "'" << std::endl;

        std::vector<std::string> filePaths = ReadLines(m_fileSystem, chunkListFileName);

        output << "Reading " << filePaths.size() << " files\n";

        IConfiguration const & configuration = index->GetConfiguration();
        IIngestor & ingestor = index->GetIngestor();

        // Create special file manager for output.
        auto fileManager = Factories::CreateFileManager(
            outputDirectory,
            outputDirectory,
            outputDirectory,
            index->GetFileSystem());

        // Select IChunkWriterFactory based on name provided on command line.
        std::unique_ptr<IChunkWriterFactory> chunkWriterFactory;
        if (!strcmp(writer, "copy"))
        {
            chunkWriterFactory = Factories::CreateCopyingChunkWriterFactory(*fileManager);
        }
        else if (!strcmp(writer, "annotate"))
        {
            chunkWriterFactory =
                Factories::CreateAnnotatingChunkWriterFactory(
                    *fileManager,
                    ingestor.GetShardDefinition());
        }
        else
        {
            FatalError error("Invalid writer. Use `copy` or `annotate`.");
            throw error;
        }

        auto manifest = Factories::CreateChunkManifestIngestor(
            m_fileSystem,
            chunkWriterFactory.get(),
            filePaths,
            configuration,
            ingestor,
            filter,
            false);

        output << "Filtering chunks . . ." << std::endl;

        Stopwatch stopwatch;

        {
            // Block scopes manifestFile.
            auto manifestFile = fileManager->Manifest().OpenForWrite();

            // DESIGN NOTE: Using single-threaded code here to avoid two
            // threads writing to manifestFile and filter.
            // If corpus filtering was performance critical and not disk
            // bound, we might consider adding synchronization to allow
            // multiple threads.
            for (size_t i = 0; i < filePaths.size(); ++i)
            {
                // Copy chunks.
                manifest->IngestChunk(i);

                // Add corresponding entry to manifest file.
                *manifestFile
                    << fileManager->Chunk(i).GetName()
                    << std::endl;
            }
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
