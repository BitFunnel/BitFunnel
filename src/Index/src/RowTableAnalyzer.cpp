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
#include <stack>

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
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Index/Token.h"
#include "CsvTsv/Csv.h"
#include "DocumentHandleInternal.h"
#include "LoggerInterfaces/Check.h"
#include "RowTableAnalyzer.h"
#include "RowTableDescriptor.h"
#include "Shard.h"
#include "Slice.h"
#include "TermToText.h"


namespace BitFunnel
{
    void Factories::AnalyzeRowTables(ISimpleIndex const & index,
                                     char const * outDir)
    {
        CHECK_NE(*outDir, '\0')
            << "Output directory not set. ";

        RowTableAnalyzer statistics(index);
        statistics.AnalyzeColumns(outDir);
        statistics.AnalyzeRows(outDir);
    }


    RowTableAnalyzer::RowTableAnalyzer(ISimpleIndex const & index)
        : m_index(index)
    {
    }


    //*************************************************************************
    //
    // Analyze rows
    //
    //*************************************************************************
    void RowTableAnalyzer::AnalyzeRows(char const * outDir) const
    {
        //
        // Gather row statistics for ingested documents.
        // (documents need not be cached)
        //
        auto & fileManager = m_index.GetFileManager();
        auto & ingestor = m_index.GetIngestor();

        // TODO: Create with factory?
        TermToText termToText(*fileManager.TermToText().OpenForRead());

        for (ShardId shardId = 0; shardId < ingestor.GetShardCount(); ++shardId)
        {
            auto terms(Factories::CreateDocumentFrequencyTable(
                *fileManager.DocFreqTable(shardId).OpenForRead()));

            auto fileSystem = Factories::CreateFileSystem();
            auto outFileManager =
                Factories::CreateFileManager(outDir,
                                             outDir,
                                             outDir,
                                             *fileSystem);

            AnalyzeRowsInOneShard(shardId,
                                  termToText,
                                  *outFileManager->RowDensities(shardId).OpenForWrite());
        }
    }


    void RowTableAnalyzer::AnalyzeRowsInOneShard(
        ShardId const & shardId,
        ITermToText const & termToText,
        std::ostream& out) const
    {
        auto & fileManager = m_index.GetFileManager();
        auto terms(Factories::CreateDocumentFrequencyTable(
            *fileManager.DocFreqTable(shardId).OpenForRead()));

        std::array<std::vector<double>, c_maxRankValue> densities;

        for (Rank rank = 0; rank < c_maxRankValue; ++rank)
        {
            auto & shard = m_index.GetIngestor().GetShard(shardId);
            densities[rank] = shard.GetDensities(rank);
        }

        // Use CsvTableFormatter to escape terms that contain commas and quotes.
        CsvTsv::CsvTableFormatter formatter(out);

        for (auto dfEntry : *terms)
        {
            Term term = dfEntry.GetTerm();
            RowIdSequence rows(term, m_index.GetTermTable());

            formatter.WriteField(termToText.Lookup(term.GetRawHash()));
            formatter.WriteField(dfEntry.GetFrequency());

            std::stack<RowId> rowsReversed;
            for (auto row : rows)
            {
                rowsReversed.push(row);
            }

            while (!rowsReversed.empty())
            {
                auto row = rowsReversed.top();
                formatter.WriteField("r");
                out << row.GetRank();
                formatter.WriteField(row.GetIndex());
                formatter.WriteField(densities[row.GetRank()][row.GetIndex()]);

                rowsReversed.pop();
            }
            formatter.WriteRowEnd();
        }
    }


    //*************************************************************************
    //
    // Analyze columns
    //
    //*************************************************************************
    void RowTableAnalyzer::AnalyzeColumns(char const * outDir) const
    {
        auto & ingestor = m_index.GetIngestor();
        auto & cache = ingestor.GetDocumentCache();

        std::vector<Column> columns;

        for (auto doc : cache)
        {
            const DocumentHandleInternal
                handle(ingestor.GetHandle(doc.second));

            // TODO: GetSlice() should return Slice&, not Slice*
            Slice const & slice = handle.GetSlice();

            const ShardId shard = slice.GetShard().GetId();

            columns.emplace_back(doc.second,
                                 shard,
                                 doc.first.GetPostingCount());

            void const * buffer = slice.GetSliceBuffer();
            const DocIndex column = handle.GetIndex();

            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                RowTableDescriptor rowTable = slice.GetRowTable(rank);

                size_t bitCount = 0;
                const size_t rowCount = rowTable.GetRowCount();
                for (RowIndex row = 0; row < rowCount; ++row)
                {
                    bitCount +=
                        rowTable.GetBit(buffer, row, column);
                }

                columns.back().SetCount(rank, bitCount);

                double density =
                    (rowCount == 0) ? 0.0 : static_cast<double>(bitCount) / rowCount;
                columns.back().SetDensity(rank, density);
            }
        }

        auto fileSystem = Factories::CreateFileSystem();
        auto outFileManager =
            Factories::CreateFileManager(outDir,
                                         outDir,
                                         outDir,
                                         *fileSystem);

        //
        // Write out raw column data.
        //
        {
            // No need to use CsvTableFormatter for escaping because all values
            // are numbers.
            auto out = outFileManager->ColumnDensities().OpenForWrite();
            Column::WriteHeader(*out);
            for (auto column : columns)
            {
                column.Write(*out);
            }
        }


        //
        // Generate summary.
        //
        {
            auto out = outFileManager->ColumnDensitySummary().OpenForWrite();

            *out << "Summary" << std::endl;

            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                Accumulator accumulator;
                for (auto column : columns)
                {
                    accumulator.Record(column.m_densities[rank]);
                }

                *out << "Rank " << rank << std::endl
                    << "  column density: " << std::endl
                    << "      min: " << accumulator.GetMin() << std::endl
                    << "      max: " << accumulator.GetMax() << std::endl
                    << "     mean: " << accumulator.GetMean() << std::endl
                    << "      var: " << accumulator.GetVariance() << std::endl
                    << "    count: " << accumulator.GetCount() << std::endl;
            }
        }

    }


    RowTableAnalyzer::Column::Column(DocId id, ShardId shard, size_t postings)
      : m_id(id),
        m_postings(postings),
        m_shard(shard),
        m_bits{}
    {
    }


    void RowTableAnalyzer::Column::SetCount(Rank rank, size_t count)
    {
        m_bits[rank] = count;
    }


    void RowTableAnalyzer::Column::SetDensity(Rank rank, double density)
    {
        m_densities[rank] = density;
    }


    void RowTableAnalyzer::Column::WriteHeader(std::ostream& out)
    {
        out << "id,postings,shard";

        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            out << ",rank" << rank;
        }

        out << std::endl;
    }


    void RowTableAnalyzer::Column::Write(std::ostream& out)
    {
        out << m_id
            << "," << m_postings
            << "," << m_shard;

        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            // TODO: Consider writing out or eliminating m_bits.
            out << "," << m_densities[rank];
        }

        out << std::endl;
    }
}
