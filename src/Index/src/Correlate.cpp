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


#include <ostream>
#include <unordered_map>                        // TODO: consider changing.
#include <unordered_set>                        // TODO: consider changing.

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowId.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Index/Token.h"
#include "BitFunnel/Term.h"
#include "Correlate.h"
#include "CsvTsv/Csv.h"
#include "DocumentHandleInternal.h"
#include "LoggerInterfaces/Check.h"
#include "NativeJIT/TypeConverter.h"
#include "RowTableAnalyzer.h"
#include "RowTableDescriptor.h"
#include "Shard.h"
#include "Slice.h"

// Define hash of RowId to allow use of map/set.
// TODO: remove this when we stop using map/set.
namespace std
{
    template<>
    struct hash<BitFunnel::RowId>
    {
        std::size_t operator()(BitFunnel::RowId const & row) const
        {
            // TODO: do we need to hash this?
            return NativeJIT::convertType<BitFunnel::RowId, size_t>(row);
        }
    };
}


namespace BitFunnel
{
    void Factories::CreateCorrelate(ISimpleIndex const & index,
                                    char const * outDir,
                                    std::vector<std::string> const & terms)
    {
        CHECK_NE(*outDir, '\0')
            << "Output directory not set. ";

        Correlate correlate(index, terms);
        correlate.CorrelateRows(outDir);
    }


    Correlate::Correlate(ISimpleIndex const & index,
                         std::vector<std::string> const & terms)
        : m_index(index),
          m_terms(terms)
    {
    }


    void Correlate::CorrelateRows(char const * outDir) const
    {
        auto & ingestor = m_index.GetIngestor();

        for (ShardId shardId = 0; shardId < ingestor.GetShardCount(); ++shardId)
        {
            auto fileSystem = Factories::CreateFileSystem();
            auto outFileManager =
                Factories::CreateFileManager(outDir,
                                             outDir,
                                             outDir,
                                             *fileSystem);

            CorrelateShard(shardId,
                           *outFileManager->Correlate(shardId).OpenForWrite());
        }
    }


    void Correlate::CorrelateShard(ShardId const & shardId,
                                   std::ostream& out) const
    {
        const Term::StreamId c_TODOStreamId = 0;
        std::unordered_map<Term::Hash, std::unordered_set<RowId>> hashToRowId;

        // auto & fileManager = m_index.GetFileManager();
        for (auto const & termText : m_terms)
        {
            Term term(termText.c_str(), c_TODOStreamId, m_index.GetConfiguration());
            RowIdSequence rows(term, m_index.GetTermTable(shardId));
            for (RowId row : rows)
            {
                hashToRowId[term.GetRawHash()].insert(row);
            }
        }


        for (auto const & outerTermText : m_terms)
        {
            Term outerTerm(outerTermText.c_str(),
                           c_TODOStreamId,
                           m_index.GetConfiguration());
            // Use CsvTableFormatter to escape terms that contain commas and quotes.
            CsvTsv::CsvTableFormatter formatter(out);

            formatter.WriteField(outerTermText);
            for (auto const & innerTermText : m_terms)
            {
                Term innerTerm(innerTermText.c_str(),
                               c_TODOStreamId,
                               m_index.GetConfiguration());
                RowIdSequence outerRows(outerTerm, m_index.GetTermTable(shardId));
                size_t intersectionSize = 0;
                for (RowId row : outerRows)
                {
                    if (hashToRowId[innerTerm.GetRawHash()].find(row) !=
                        hashToRowId[innerTerm.GetRawHash()].end())
                    {
                        ++intersectionSize;
                    }
                }
                if (intersectionSize > 1 &&
                    innerTermText != outerTermText)
                {
                    formatter.WriteField(innerTermText);
                    formatter.WriteField(intersectionSize);
                }
            }
            out << std::endl;
        }
    }
}
