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

// TODO: port this test.

#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/RowId.h"
#include "PackedArray.h"
#include "SuiteCpp/UnitTest.h"

namespace BitFunnel
{
    namespace RowIdUnitTest
    {
        class UnitTestRowId : public RowId
        {
        public:
            static unsigned GetShardBitCount();
            static unsigned GetTierBitCount();
            static unsigned GetRankBitCount();
            static unsigned GetIndexBitCount();
        };


        unsigned UnitTestRowId::GetShardBitCount()
        {
            return c_bitsOfShard;
        }


        unsigned UnitTestRowId::GetTierBitCount()
        {
            return c_bitsOfTier;
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
        // ShardId, Tier, Rank, and RowIndex values.
        TestCase(Limits)
        {
            TestAssert(c_maxShardCount <= (1ul << UnitTestRowId::GetShardBitCount()));
            TestAssert(TierCount <= (1ul << UnitTestRowId::GetTierBitCount()));
            TestAssert(c_maxRankValue <= (1ul << UnitTestRowId::GetRankBitCount()) - 1);
            TestAssert(RowId::c_maxRowIndexValue <= (1ul << UnitTestRowId::GetIndexBitCount()) - 1);
            TestAssert(RowId::GetPackedRepresentationBitCount() <= PackedArray::GetMaxBitsPerEntry());
        }


        // Verify that field getters return field values passed to constructor.
        TestCase(Fields)
        {
            for (ShardId shard = 0; shard < c_maxShardCount; ++ shard)
            {
                for (unsigned t = 0; t < TierCount; ++t)
                {
                    for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
                    {
                        for (int i = -1; i <= 1; ++i)
                        {
                            Tier tier = Tier(t);
                            RowIndex index = static_cast<unsigned>(i) & (1ul << UnitTestRowId::GetIndexBitCount()) - 1;

                            RowId rowId(shard, tier, rank, index);

                            TestAssert(rowId.GetShard() == shard);
                            TestAssert(rowId.GetTier() == tier);
                            TestAssert(rowId.GetRank() == rank);
                            TestAssert(rowId.GetIndex() == index);
                        }
                    }
                }
            }
        }


        // Verify packed representation round-tripping.
        TestCase(PackedFormat)
        {
            for (ShardId shard = 0; shard < c_maxShardCount; ++ shard)
            {
                for (unsigned t = 0; t < TierCount; ++t)
                {
                    for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
                    {
                        for (int i = -1; i <= 1; ++i)
                        {
                            Tier tier = Tier(t);
                            RowIndex index = static_cast<unsigned>(i) & (1ul << UnitTestRowId::GetIndexBitCount()) - 1;

                            RowId rowId1(shard, tier, rank, index);
                            unsigned __int64 packed = rowId1.GetPackedRepresentation();

                            RowId rowId2(packed);

                            TestAssert(rowId2.GetShard() == shard);
                            TestAssert(rowId2.GetTier() == tier);
                            TestAssert(rowId2.GetRank() == rank);
                            TestAssert(rowId2.GetIndex() == index);
                        }
                    }
                }
            }
        }
    }
}
