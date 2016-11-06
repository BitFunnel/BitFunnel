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

#include "BitFunnel/Chunks/DocumentFilters.h"
#include "BitFunnel/Chunks/Factories.h"
#include "BitFunnel/Chunks/IChunkManifestIngestor.h"
#include "BitFunnel/Chunks/IChunkProcessor.h"
#include "BitFunnel/Data/Sonnets.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "Environment.h"
#include "IngestCommands.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Ingest
    //
    //*************************************************************************
    Ingest::Ingest(Environment & environment,
                   Id id,
                   char const * parameters,
                   bool cacheDocuments)
        : TaskBase(environment, id, Type::Synchronous),
        m_cacheDocuments(cacheDocuments)
    {
        auto command = TaskFactory::GetNextToken(parameters);
        if (command.compare("manifest") == 0)
        {
            m_manifest = true;
        }
        else if (command.compare("chunk") == 0)
        {
            m_manifest = false;
        }
        else
        {
            RecoverableError error("Ingest expects \"chunk\" or \"manifest\".");
            throw error;
        }

        m_path = TaskFactory::GetNextToken(parameters);
    }


    void Ingest::Execute()
    {
        if (m_manifest && m_path.compare("sonnets") == 0)
        {
            Environment & environment = GetEnvironment();
            IConfiguration const & configuration =
                environment.GetConfiguration();
            IIngestor & ingestor = environment.GetIngestor();
            size_t threadCount = 1;

            //auto factory = Factories::CreateChunkIngestorFactory(
            //    configuration,
            //    ingestor,
            //    m_cacheDocuments);

            auto manifest = Factories::CreateBuiltinChunkManifest(
                Sonnets::chunks,
                configuration,
                ingestor,
                m_cacheDocuments);

            IngestChunks(*manifest, threadCount);

            std::cout << "Ingestion complete." << std::endl;
        }
        else
        {
            std::vector<std::string> filePaths;

            if (m_manifest)
            {
                std::cout
                    << "Ingesting manifest \""
                    << m_path
                    << "\"" << std::endl;

                filePaths = ReadLines(GetEnvironment().GetFileSystem(),
                                      m_path.c_str());

            }
            else
            {
                filePaths.push_back(m_path);
                std::cout
                    << "Ingesting chunk file \""
                    << filePaths.back()
                    << "\"" << std::endl;
            }

            if (m_cacheDocuments)
            {
                std::cout
                    << "Caching IDocuments for query verification."
                    << std::endl;
            }

            Environment & environment = GetEnvironment();
            IFileSystem & fileSystem =
                environment.GetFileSystem();
            IConfiguration const & configuration =
                environment.GetConfiguration();
            IIngestor & ingestor = environment.GetIngestor();
            size_t threadCount = 1;

            //auto factory = Factories::CreateChunkIngestorFactory(
            //    configuration,
            //    ingestor,
            //    m_cacheDocuments);

            NopFilter filter;

            auto manifest = Factories::CreateChunkManifestIngestor(
                fileSystem,
                nullptr,
                filePaths,
                configuration,
                ingestor,
                filter,
                m_cacheDocuments);

            IngestChunks(*manifest, threadCount);

            std::cout << "Ingestion complete." << std::endl;
        }
    }


    ICommand::Documentation Ingest::GetDocumentation()
    {
        return Documentation(
            "ingest",
            "Ingests documents into the index. (TODO)",
            "ingest (manifest | chunk) <path>\n"
            "  Ingests a single chunk file or a list of chunk\n"
            "  files specified by a manifest.\n"
            "  NOT IMPLEMENTED"
        );
    }


    //*************************************************************************
    //
    // Cache
    //
    //*************************************************************************
    Cache::Cache(Environment & environment,
                 Id id,
                 char const * parameters)
        : Ingest(environment, id, parameters, true)
    {
    }


    ICommand::Documentation Cache::GetDocumentation()
    {
        return Documentation(
            "cache",
            "Ingests documents into the index and also stores them in a cache\n"
            "for query verification purposes.",
            "cache (manifest | chunk) <path>\n"
            "  Ingests a single chunk file or a list of chunk\n"
            "  files specified by a manifest.\n"
            "  Also caches IDocuments for query verification.\n"
        );
    }


    //*************************************************************************
    //
    // Load
    //
    //*************************************************************************
    Load::Load(Environment & environment,
               Id id,
               char const * parameters)
        : Ingest(environment, id, parameters, false)
    {
    }


    ICommand::Documentation Load::GetDocumentation()
    {
        return Documentation(
            "load",
            "Ingests documents into the index",
            "load (manifest | chunk) <path>\n"
            "  Ingests a single chunk file or a list of chunk\n"
            "  files specified by a manifest.\n"
        );
    }
}
