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

#pragma once

#include <array>                                // std::arrray embedded.
#include <iosfwd>                               // std::ostream paramter.
#include <vector>                               // std::vector embedded.

#include "BitFunnel/BitFunnelTypes.h"           // DocId embedded.
#include "BitFunnel/Utilities/Accumulator.h"    // Accumulator embedded.


namespace BitFunnel
{
    class IFileManager;
    class ISimpleIndex;
    class ITermToText;

    class RowTableAnalyzer
    {
    public:
        RowTableAnalyzer(ISimpleIndex const & index);

        void AnalyzeRows(char const * outDir) const;
        void AnalyzeColumns(char const * outDir) const;

    private:
        void AnalyzeRowsInOneShard(
            ShardId const & shardId,
            ITermToText const & termToText,
            std::ostream& out,
            std::ostream& summaryOut) const;

        class Column
        {
        public:
            Column(DocId id, ShardId shard, size_t postings);

            void SetCount(Rank rank, size_t bitCount);
            void SetDensity(Rank rank, double density);

            static void WriteHeader(std::ostream& out);
            void Write(std::ostream& out);

            DocId m_id;
            size_t m_postings;
            ShardId m_shard;
            std::array<size_t, c_maxRankValue + 1> m_bits;
            std::array<double, c_maxRankValue + 1> m_densities;
        };

        ISimpleIndex const & m_index;
    };
}
