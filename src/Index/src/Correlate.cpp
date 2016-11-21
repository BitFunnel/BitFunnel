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
#include "TermToText.h"


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

        auto & fileManager = m_index.GetFileManager();
        TermToText termToText(*fileManager.TermToText().OpenForRead());
        for (ShardId shardId = 0; shardId < ingestor.GetShardCount(); ++shardId)
        {
            auto fileSystem = Factories::CreateFileSystem();
            auto outFileManager =
                Factories::CreateFileManager(outDir,
                                             outDir,
                                             outDir,
                                             *fileSystem);

            CorrelateShard(shardId,
                           termToText,
                           *outFileManager->Correlate(shardId).OpenForWrite());
        }
    }


    void Correlate::CorrelateShard(ShardId const & shardId,
                                   ITermToText const & termToText,
                                   std::ostream& out) const
    {
        const Term::StreamId c_TODOStreamId = 0;
        // TODO: this should be a vector instead of a set. That change shouldn't
        // really affect running time much, though, since we that should be
        // dominated by the later phases.
        std::unordered_map<RowId, std::unordered_set<Term::Hash>> rowIdToHash;

        // auto & fileManager = m_index.GetFileManager();
        for (auto const & termText : m_terms)
        {
            Term term(termText.c_str(), c_TODOStreamId, m_index.GetConfiguration());
            RowIdSequence rows(term, m_index.GetTermTable(shardId));
            for (RowId row : rows)
            {
                rowIdToHash[row].insert(term.GetRawHash());
            }
        }


        std::unordered_map<Term::Hash,
                           std::unordered_map<Term::Hash, uint8_t>> collisions;
        for (auto const & item : rowIdToHash)
        {
            auto const & rowId = item.first;
            auto const & hashes = rowIdToHash[rowId];
            for (auto const & leftHash : hashes)
            {
                for (auto const & rightHash : hashes)
                {
                    if (leftHash != rightHash)
                    {
                        ++collisions[leftHash][rightHash];
                    }
                }
            }
        }

        CsvTsv::CsvTableFormatter formatter(out);
        for (auto const & item : collisions)
        {
            auto const & leftHash = item.first;
            // TODO: consider only writing out terms with collisions.
            formatter.WriteField(termToText.Lookup(leftHash));
            for (auto const & innerItem : item.second)
            {
                auto const & rightHash = innerItem.first;
                const uint8_t numCollisions = innerItem.second;
                if (numCollisions > 1)
                {
                    formatter.WriteField(termToText.Lookup(rightHash));
                    formatter.WriteField(numCollisions);
                }
            }
            formatter.WriteRowEnd();
        }
    }
}
