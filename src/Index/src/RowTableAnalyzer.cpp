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
#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "CsvTsv/Csv.h"
#include "DocumentHandleInternal.h"
#include "LoggerInterfaces/Check.h"
#include "RowTableAnalyzer.h"
#include "RowTableDescriptor.h"
#include "Shard.h"
#include "TermToText.h"


namespace BitFunnel
{
    void Factories::AnalyzeRowTables(ISimpleIndex const & index,
                                     char const * outDir)
    {
        char const end = '\0';     // TODO: Workaround for issue #386.
        CHECK_NE(*outDir, end)
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
    // Row Statistics
    //
    //*************************************************************************
    class RowStatistics
    {
    public:
        static void WriteHeader(std::ostream& out)
        {
            CsvTsv::CsvTableFormatter formatter(out);
            formatter.WritePrologue(std::vector<char const *>{
                "shard", "idfX10", "rank", "mean", "min", "max", "var", "count"});
        }


        void WriteSummary(std::ostream& out, 
                          ShardId const & shardId,
                          Term::IdfX10 idfX10) const
        {
            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                Accumulator const & accumulator = m_accumulators[rank];
                if (accumulator.GetCount() == 0)
                    continue;

                out << shardId << ","
                    << static_cast<uint32_t>(idfX10) << ","
                    << rank << ","
                    << accumulator.GetMean() << ","
                    << accumulator.GetMin() << ","
                    << accumulator.GetMax() << ","
                    << accumulator.GetVariance() << ","
                    << accumulator.GetCount() << std::endl;
            }
        }


        void RecordDensity(Rank rank, double density)
        {
            m_accumulators.at(rank).Record(density);
        }


        void Reset()
        {
            for (auto it = m_accumulators.begin(); it != m_accumulators.end(); ++it)
            {
                it->Reset();
            }
        }

    private:
        std::array<Accumulator, c_maxRankValue + 1> m_accumulators;
    };


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

        // Obtain data for converting a term hash to text, if file exists
        std::unique_ptr<ITermToText> termToText;
        try
        {
            // Will fail with exception if file does not exist or can't be read
            // TODO: Create with factory?
            termToText = Factories::CreateTermToText(*fileManager.TermToText().OpenForRead());
        }
        catch (RecoverableError e)
        {
            termToText = Factories::CreateTermToText();
        }

        auto fileSystem = Factories::CreateFileSystem();
        auto outFileManager =
            Factories::CreateFileManager(outDir,
                outDir,
                outDir,
                *fileSystem);

        auto summaryOut = outFileManager->RowDensitySummary().OpenForWrite();
        RowStatistics::WriteHeader(*summaryOut);

        for (ShardId shardId = 0; shardId < ingestor.GetShardCount(); ++shardId)
        {
            AnalyzeRowsInOneShard(shardId,
                                  *termToText,
                                  *outFileManager->RowDensities(shardId).OpenForWrite(),
                                  *summaryOut);
        }
    }


    void RowTableAnalyzer::AnalyzeRowsInOneShard(
        ShardId const & shardId,
        ITermToText const & termToText,
        std::ostream& out,
        std::ostream& summaryOut) const
    {
        auto & shard = m_index.GetIngestor().GetShard(shardId);
        std::array<std::vector<double>, c_maxRankValue + 1> densities;

        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            densities[rank] = shard.GetDensities(rank);
        }

        RowStatistics statistics;
        Term::IdfX10 curIdfX10 = 0;

        // Use CsvTableFormatter to escape terms that contain commas and quotes.
        CsvTsv::CsvTableFormatter formatter(out);
        auto & fileManager = m_index.GetFileManager();
        auto terms(Factories::CreateDocumentFrequencyTable(
            *fileManager.DocFreqTable(shardId).OpenForRead()));

        for (auto dfEntry : *terms)
        {
            Term term = dfEntry.GetTerm();
            Term::IdfX10 idfX10 = Term::ComputeIdfX10(dfEntry.GetFrequency(), Term::c_maxIdfX10Value);

            // When we cross an idfX10 bucket boundary, output statistics
            // accumulated since previous idfX10 bucket. Then reset
            // statistics accumulators for the next bucket.
            if (idfX10 != curIdfX10)
            {
                curIdfX10 = idfX10;
                statistics.WriteSummary(summaryOut, shardId, curIdfX10);
                statistics.Reset();
            }

            RowIdSequence rows(term, m_index.GetTermTable(shardId));

            std::string termName = termToText.Lookup(term.GetRawHash());
            if (!termName.empty())
            {
                // Print out the term text if we have it.
                formatter.WriteField(termName);
            }
            else
            {
                // Otherwise print out the term's hash.
                formatter.SetHexMode(1);
                formatter.WriteField(term.GetRawHash());
                formatter.SetHexMode(0);
            }

            formatter.WriteField(dfEntry.GetFrequency());

            std::stack<RowId> rowsReversed;
            for (auto row : rows)
            {
                rowsReversed.push(row);
            }

            while (!rowsReversed.empty())
            {
                auto row = rowsReversed.top();
                auto rank = row.GetRank();
                auto index = row.GetIndex();
                auto density = densities.at(rank).at(index);
                statistics.RecordDensity(rank, density);

                formatter.WriteField("r");
                out << row.GetRank();
                formatter.WriteField(index);
                formatter.WriteField(density);

                rowsReversed.pop();
            }
            formatter.WriteRowEnd();
        }
        statistics.WriteSummary(summaryOut, shardId, curIdfX10);
    }


    //*************************************************************************
    //
    // Analyze columns
    //
    //*************************************************************************
    void RowTableAnalyzer::AnalyzeColumns(char const * outDir) const
    {
        auto & ingestor = m_index.GetIngestor();

        auto fileSystem = Factories::CreateFileSystem();
        auto outFileManager =
            Factories::CreateFileManager(outDir,
                                         outDir,
                                         outDir,
                                         *fileSystem);

        auto summaryOut = outFileManager->ColumnDensitySummary().OpenForWrite();

        // TODO: Consider using CsvTsv::CsvTableFormatter::WritePrologue() here.
        *summaryOut << "shard,rank,mean,min,max,var,count" << std::endl;

        std::array<Accumulator, c_maxRankValue + 1> accumulators;

        for (ShardId shardId = 0; shardId < ingestor.GetShardCount(); ++shardId)
        {
            IShard& shard = ingestor.GetShard(shardId);

            // Write out each document's raw column data per shard.
            // No need to use CsvTableFormatter for escaping because all values
            // are numbers.
            auto out = outFileManager->ColumnDensities(shardId).OpenForWrite();
            Column::WriteHeader(*out);

            auto itPtr = shard.GetIterator();
            auto & it = *itPtr;

            for (; !it.AtEnd(); ++it)
            {
                const DocumentHandleInternal handle(*it);
                const DocId docId = handle.GetDocId();
                Slice const & slice = handle.GetSlice();

                // TODO: handle.GetPostingCount() instead of 0
                Column column(docId, shardId, 0);

                void const * buffer = slice.GetSliceBuffer();
                const DocIndex docIndex = handle.GetIndex();

                for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
                {
                    RowTableDescriptor rowTable = slice.GetRowTable(rank);

                    size_t bitCount = 0;
                    const size_t rowCount = rowTable.GetRowCount();
                    for (RowIndex row = 0; row < rowCount; ++row)
                    {
                        if (rowTable.GetBit(buffer, row, docIndex) != 0)
                        {
                            ++bitCount;
                        }
                    }

                    column.SetCount(rank, bitCount);

                    double density =
                        (rowCount == 0) ? 0.0 : static_cast<double>(bitCount) / rowCount;
                    column.SetDensity(rank, density);
                    accumulators[rank].Record(density);
                }

                column.Write(*out);
            }

            //
            // Generate document summary by rank for shard
            //
            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                Accumulator accumulator = accumulators[rank];
                if (accumulator.GetCount() == 0)
                    continue;

                *summaryOut
                    << shardId << ","
                    << rank << ","
                    << accumulator.GetMean() << ","
                    << accumulator.GetMin() << ","
                    << accumulator.GetMax() << ","
                    << accumulator.GetVariance() << ","
                    << accumulator.GetCount() << std::endl;

                accumulators[rank].Reset();
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
