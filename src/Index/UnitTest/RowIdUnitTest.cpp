#include "stdafx.h"

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
