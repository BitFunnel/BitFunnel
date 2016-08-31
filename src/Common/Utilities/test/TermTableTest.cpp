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


// TODO: port.
#include "stdafx.h"

#include "Array.h"
#include "BitFunnel/Factories.h"
#include "BitFunnel/IFactSet.h"
#include "BitFunnel/ITermDisposeDefinition.h"
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/PackedTermInfo.h"
#include "BitFunnel/Term.h"
#include "BitFunnel/TermInfo.h"
#include "Random.h"
#include "SuiteCpp/Test.h"


namespace BitFunnel
{
    namespace TermTableTest
    {
        static char * const c_anyFactName = "AnyFactName";
        static char * const c_anyOtherFactName = "AnyOtherFactName";

        struct TermTableEntry
        {
            TermTableEntry(Term const & term);

            Term m_term;
            bool m_isPrivate;
            bool m_isAdhoc;

            unsigned m_rows[c_maxRankValue + 1];
        };


        TermTableEntry::TermTableEntry(Term const & term)
            : m_term(term)
        {
        }


        static const ShardId c_shard = 0;

        // Verifies that a TermInfo which corresponds to a fact has the correct
        // rows by consulting with the given ITermTable.
        void CheckFactRow(TermInfo& termInfo,
                          RowIndex expectedRowIndex)
        {
            TestAssert(!termInfo.IsEmpty());
            TestAssert(termInfo.MoveNext());
            const RowId row = termInfo.Current();

            TestEqual(row.GetShard(), c_shard);
            TestEqual(row.GetTier(), DDRTier);
            TestEqual(row.GetRank(), 0u);
            TestEqual(row.GetIndex(), expectedRowIndex);

            TestAssert(!termInfo.MoveNext());
        }


        TestCase(TermTableWithFactsTest)
        {
            static const char* termText[] = {
                "This", "is", "a", "test", "for", "posting", "list", "builder",
                "verifier", "powered", "by", "smf", "you", "tube", nullptr };

            std::stringstream termDisposeDefinitionStream;
            termDisposeDefinitionStream << "GramSize,DDRIdfSumThreshold,SSDIdfSumThreshold,HDDIdfSumThreshold\n"
                "1, 254, 254, 254\n"
                "2, 254, 254, 254\n"
                "3, 254, 254, 254\n"
                "4, 254, 254, 254\n"
                "5, 254, 254, 254\n"
                "6, 254, 254, 254\n"
                "7, 254, 254, 254\n"
                "8, 254, 254, 254\n";
            std::unique_ptr<const ITermDisposeDefinition> termDispose(
                Factories::CreateTermDisposeDefinition(termDisposeDefinitionStream));

            RandomInt<unsigned> random(123456, 0, 1000);
            std::vector<TermTableEntry> entries;
            unsigned rowCount = 0;
            for (unsigned i = 0; termText[i] != nullptr; i++)
            {
                // Randomly disperse across adhoc and private terms.
                const bool isAdhoc = (random() % 10) > 7;
                const bool isPrivate = !isAdhoc && ((random() % 10) > 6);

                const Term term(Term::ComputeRawHash(termText[i]),
                                static_cast<Stream::Classification>(random() & Stream::ClassificationCount),
                                static_cast<IdfX10>(random() % c_maxIdfX10Value),
                                static_cast<Tier>(random() % TierCount));
                TermTableEntry entry(term);
                entry.m_isPrivate = isPrivate;
                entry.m_isAdhoc = isAdhoc;

                if (isPrivate)
                {
                    entry.m_rows[0] = 1;
                }
                else
                {
                    entry.m_rows[0] = random() % 3;
                    entry.m_rows[3] = random() % 5;
                    entry.m_rows[6] = random() % 5;
                }

                rowCount += entry.m_rows[0] + entry.m_rows[3] + entry.m_rows[6];

                entries.push_back(entry);
            }

            typedef Array2DFixed<unsigned, TierCount, c_maxRankValue + 1>
                UnsignedRankTierArray;

            UnsignedRankTierArray rowIndexCounter;
            UnsignedRankTierArray privateRowCount;
            UnsignedRankTierArray sharedRowCount;

            std::unique_ptr<ITermTable> termTable1(Factories::CreateTermTable(
                termDispose.get(), static_cast<unsigned>(entries.size()), rowCount));

            for (const auto& entry : entries)
            {
                unsigned rowIdOffset = termTable1->GetRowIdCount();

                for (Rank rank = 0; rank <= c_maxRankValue; rank += 3)
                {
                    for (unsigned row = 0; row < entry.m_rows[rank]; ++row)
                    {
                        const RowIndex rowIndex = rowIndexCounter.At(entry.m_term.GetTierHint(), rank)++;
                        termTable1->AddRowId(RowId(c_shard, entry.m_term.GetTierHint(), rank, rowIndex));

                        if (entry.m_isPrivate)
                        {
                            privateRowCount.At(entry.m_term.GetTierHint(), rank)++;
                        }
                        else
                        {
                            sharedRowCount.At(entry.m_term.GetTierHint(), rank)++;
                        }
                    }
                }

                if (entry.m_isAdhoc)
                {
                    termTable1->AddTermAdhoc(entry.m_term.GetClassification(),
                                             1,
                                             entry.m_term.GetTierHint(),
                                             entry.m_term.GetIdfSum(),
                                             rowIdOffset,
                                             entry.m_rows[0] + entry.m_rows[3] + entry.m_rows[6]);
                }
                else
                {
                    termTable1->AddTerm(entry.m_term.GetClassifiedHash(),
                                        rowIdOffset,
                                        entry.m_rows[0] + entry.m_rows[3] + entry.m_rows[6]);
                }
            }

            for (unsigned t = 0; t < TierCount; ++t)
            {
                for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
                {
                    termTable1->SetRowTableSize(static_cast<Tier>(t),
                                                rank,
                                                privateRowCount.At(t, rank),
                                                sharedRowCount.At(t, rank));
                }
            }

            // Output a term table to a stream in order to create another one
            // from its data.
            std::stringstream s;
            termTable1->Write(s);

            // Create another ITermTable with a number of pre-defined facts.
            std::unique_ptr<IFactSet> facts(Factories::CreateFactSet());
            facts->DefineFact(c_anyFactName, false);
            facts->DefineFact(c_anyOtherFactName, false);

            const unsigned factsCount = facts->GetCount();
            TestEqual(factsCount, 2u);

            std::unique_ptr<ITermTable const> termTable2(Factories::CreateTermTable(s, *facts));

            // For previously stored terms, all row assignment should be unchanged.
            for (const auto& entry : entries)
            {
                TermInfo termInfo1(entry.m_term, *termTable1);
                TermInfo termInfo2(entry.m_term, *termTable2);

                TestAssert(termInfo1.IsEmpty() == termInfo2.IsEmpty());

                if (!termInfo1.IsEmpty())
                {
                    for (;;)
                    {
                        const bool term1HasMore = termInfo1.MoveNext();
                        const bool term2HasMore = termInfo2.MoveNext();

                        TestAssert(term1HasMore == term2HasMore);

                        if (term1HasMore)
                        {
                            const RowId row1 = termInfo1.Current();
                            const RowId row2 = termInfo2.Current();

                            TestEqual(row1.GetShard(), row2.GetShard());
                            TestEqual(row1.GetTier(), row2.GetTier());
                            TestEqual(row1.GetRank(), row2.GetRank());
                            TestEqual(row1.GetIndex(), row2.GetIndex());
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            // Verify RowId assignment for soft-deleted row and facts.
            const unsigned rowCountDdr0 = privateRowCount.At(DDRTier, 0)
                                          + sharedRowCount.At(DDRTier, 0);

            {
                TermInfo termInfo(ITermTable::GetDocumentActiveTerm(), *termTable2);
                CheckFactRow(termInfo, rowCountDdr0);
            }

            {
                TermInfo termInfo(ITermTable::GetMatchAllTerm(), *termTable2);
                CheckFactRow(termInfo, rowCountDdr0 + 1);
            }

            {
                TermInfo termInfo(ITermTable::GetMatchNoneTerm(), *termTable2);
                CheckFactRow(termInfo, rowCountDdr0 + 2);
            }


            // System internal rows are soft-deleted, match-all and match-none.
            // By convention, system rows are placed before user-defined facts, hence
            // user-defined facts are offset by systemRowCount in the verification below.
            for (unsigned i = 0; i < facts->GetCount(); ++i)
            {
                TermInfo termInfo((*facts)[i], *termTable2);
                CheckFactRow(termInfo,
                             rowCountDdr0 + c_systemRowCount + i);
            }
        }
    }
}
