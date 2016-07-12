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

#include "gtest/gtest.h"

#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/RowId.h"
#include "PackedArray.h"

namespace BitFunnel
{
    namespace RowIdUnitTest
    {
        class UnitTestRowId : public RowId
        {
        public:
            static unsigned GetShardBitCount();
            static unsigned GetRankBitCount();
            static unsigned GetIndexBitCount();
        };


        unsigned UnitTestRowId::GetShardBitCount()
        {
            return c_bitsOfShard;
        }


        unsigned UnitTestRowId::GetRankBitCount()
        {
            return c_bitsOfRank;
        }


        unsigned UnitTestRowId::GetIndexBitCount()
        {
            return c_bitsOfIndex;
        }


        // Verify that the bit fields in RowId are large enough to hold all legal
        // ShardId, Rank, and RowIndex values.
        TEST(Limits, Trivial)
        {
            EXPECT_LE(c_maxShardCount, (1ul << UnitTestRowId::GetShardBitCount()));
            EXPECT_LE(c_maxRankValue, (1ul << UnitTestRowId::GetRankBitCount()) - 1);
            EXPECT_LE(RowId::c_maxRowIndexValue, (1ul << UnitTestRowId::GetIndexBitCount()) - 1);
            EXPECT_LE(RowId::GetPackedRepresentationBitCount(), PackedArray::GetMaxBitsPerEntry());
        }


        // Verify that field getters return field values passed to constructor.
        TEST(Fields, Trivial)
        {
            for (ShardId shard = 0; shard < c_maxShardCount; ++ shard)
            {
                    for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
                    {
                        for (int i = -1; i <= 1; ++i)
                        {
                            RowIndex index = static_cast<unsigned>(i) & (1ul << UnitTestRowId::GetIndexBitCount()) - 1;

                            RowId rowId(shard, rank, index);

                            EXPECT_EQ(rowId.GetShard(), shard);
                            EXPECT_EQ(rowId.GetRank(), rank);
                            EXPECT_EQ(rowId.GetIndex(), index);
                        }
                    }
            }
        }


        // Verify packed representation round-tripping.
        TEST(PackedFormat, Trivial)
        {
            for (ShardId shard = 0; shard < c_maxShardCount; ++ shard)
            {
                    for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
                    {
                        for (int i = -1; i <= 1; ++i)
                        {
                            RowIndex index = static_cast<unsigned>(i) & (1ul << UnitTestRowId::GetIndexBitCount()) - 1;

                            RowId rowId1(shard, rank, index);
                            uint64_t packed = rowId1.GetPackedRepresentation();

                            RowId rowId2(packed);

                            EXPECT_EQ(rowId2.GetShard(), shard);
                            EXPECT_EQ(rowId2.GetRank(), rank);
                            EXPECT_EQ(rowId2.GetIndex(), index);
                        }
                    }
            }
        }
    }
}
