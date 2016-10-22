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

#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "DocumentHandleInternal.h"
#include "LoggerInterfaces/Check.h"
#include "RowTableDescriptor.h"
#include "RowTableStatistics.h"
#include "Slice.h"


namespace BitFunnel
{
    RowTableStatistics::RowTableStatistics(ISimpleIndex & index)
    {
        auto & ingestor = index.GetIngestor();
        auto & cache = ingestor.GetDocumentCache();

        for (auto doc : cache)
        {
            const DocumentHandleInternal handle(ingestor.GetHandle(doc.second));

            // TODO: GetSlice() should return Slice&, not Slice*
            Slice const * slice = handle.GetSlice();

            // TODO: Implement slice->GetShard().GetId().
            const ShardId shard = 0;

            m_columns.emplace_back(doc.second,
                                   shard,
                                   doc.first.GetPostingCount());

            void * buffer = slice->GetSliceBuffer();
            const DocIndex column = handle.GetIndex();

            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                RowTableDescriptor rowTable = slice->GetRowTable(rank);

                size_t bitCount = 0;
                const size_t rowCount = rowTable.GetRowCount();
                for (RowIndex row = 0; row < rowCount; ++row)
                {
                    // TODO: GetBit should take void const *.
                    // TODO: buffer should be void const *
                    bitCount += 
                        rowTable.GetBit(buffer, row, column);
                }

                m_columns.back().SetCount(rank, bitCount);

                double density = static_cast<double>(bitCount) / rowCount;
                m_columns.back().SetDensity(rank, density);
            }
        }
    }


    void RowTableStatistics::Write(std::ostream& out) const
    {
        for (auto column : m_columns)
        {
            column.Write(out);
        }
    }


    void RowTableStatistics::Summarize(std::ostream& out) const
    {
        out << "Summary" << std::endl;
    }


    RowTableStatistics::Column::Column(DocId id, ShardId shard, size_t postings)
      : m_id(id),
        m_shard(shard),
        m_postings(postings),
        m_bits{}
    {
    }


    void RowTableStatistics::Column::SetCount(Rank rank, size_t count)
    {
        m_bits[rank] = count;
    }


    void RowTableStatistics::Column::SetDensity(Rank rank, double density)
    {
        m_densities[rank] = density;
    }


    void RowTableStatistics::Column::Write(std::ostream& out)
    {
        out << m_id
            << "," << m_postings;

        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            // TODO: Consider writing out or eliminating m_bits.
            out << "," << m_densities[rank];
        }

        out << std::endl;
    }
}
