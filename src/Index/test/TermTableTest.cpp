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

#include <sstream>

#include "gtest/gtest.h"

#include "BitFunnel/Index/RowIdSequence.h"
#include "TermTable.h"


namespace BitFunnel
{
    namespace TableTest
    {
        //*********************************************************************
        //
        // Test explicit rows.
        //
        //*********************************************************************

        // Configure a TermTable with a set of Explict Terms and then verify
        // that RowIdSequence enumerates the correct RowIds for each term.
        TEST(TermTable, ExplicitRows)
        {
            const size_t termCount = 5;
            const size_t explicitRowCount = 100;
            const size_t adhocRowCount = 200;

            // Start hash at 1000 to be well above the hashes reserved for system rows and facts.
            const Term::Hash c_firstHash = 1000ull;

            TermTable termTable;

            // Add a bunch of Explicit terms.
            for (size_t i = 0; i < termCount; ++i)
            {
                Term::Hash hash = i + c_firstHash;

                // For this test, the number of rows for each term is based on
                // its hash.
                termTable.OpenTerm();
                for (size_t r = 0; r <= (i % 3); ++r)
                {
                    termTable.AddRowId(RowId(0, r));
                }
                termTable.CloseTerm(hash);
            }

            // Finish configuring the TermTable.
            termTable.SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable.SetFactCount(0);
            termTable.Seal();

            // Verify rows for each of the terms.
            for (size_t i = 0; i < termCount; ++i)
            {
                Term::Hash hash = i + c_firstHash;

                Term term(hash, 0, 0);
                RowIdSequence rows(term, termTable);
                auto it = rows.begin();

                for (size_t r = 0; r <= (i % 3); ++r, ++it)
                {
                    // TermTable::Seal() relocates Explicit rows to the block
                    // immediately after the Adhoc rows.
                    // Expect RowId to be offset by adhocRowCount from the
                    // from the RowId passed to AddRowId().
                    RowId expected = RowId(0, r + adhocRowCount);
                    RowId observed = *it;
                    EXPECT_EQ(expected, observed);
                }

                // Verify that iterator is now at the end. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                EXPECT_TRUE(it == rows.end());
            }
        }


        //*********************************************************************
        //
        // Test adhoc rows.
        //
        //*********************************************************************

        // Verifies that the RowIdSequence for term in termTable. Checks
        // include:
        //   Number of RowIds exactly match TermTable configuration.
        //   Rank values exactly match TermTable configuration.
        //   All RowIndex values for a particular term are unique.
        //   All RowIndex values are in range according to TermTable.
        static void VerifyAdhocRowIds(
            Term term,
            TermTable const & termTable,
            std::vector<RowId> const & recipe,
            std::vector<RowIndex> const & adhocRowCounts)
        {
            // Get the RowId sequence from the TermTable.
            RowIdSequence rows(term, termTable);
            auto o = rows.begin();

            // Recipe has the expected RowIds.
            auto e = recipe.begin();

            // Set "indexes" will be used to ensure that all RowIndex
            // values for this term are unique.
            std::set<RowIndex> indexes;

            // For each row in the RowIdSequence
            while (o != rows.end())
            {
                RowId observed = *o;
                RowId expected = *e;

                // Ensure that we haven't seen this RowIndex before for
                // this term. NOTE: Cannot use EXPECT_EQ with iterators.
                auto it = indexes.find(observed.GetIndex());
                EXPECT_TRUE(it == indexes.end());

                // Record this RowIndex to ensure that it isn't repeated
                // for this term.
                indexes.insert(observed.GetIndex());

                // Ensure that Rank is correct.
                EXPECT_EQ(observed.GetRank(), expected.GetRank());

                // Ensure that RowIndex is in range.
                EXPECT_LE(observed.GetIndex(), adhocRowCounts[expected.GetRank()]);

                ++o;
                ++e;
            }
            // Verify that e has as many RowIds as o.
            // NOTE: Cannot use EXPECT_EQ with iterators.
            EXPECT_TRUE(e == recipe.end());
        }


        // Verifies that the RowIdSequences for two terms with the same IdfX10
        // and GramSize values but different hashes are different.
        static void VerifyTwoTerms(Term term1,
                                   TermTable const & termTable)
        {
            Term term2(term1.GetRawHash() + 17,
                       term1.GetStream(),
                       term1.GetIdfMax(),
                       term1.GetGramSize());

            RowIdSequence rs1(term1, termTable);
            RowIdSequence rs2(term2, termTable);

            auto it2 = rs2.begin();
            for (auto r1 : rs1)
            {
                auto r2 = *it2;

                EXPECT_EQ(r1.GetRank(), r2.GetRank());
                EXPECT_NE(r1.GetIndex(), r2.GetIndex());

                ++it2;
            }
            // Verify that rs1 has as many RowIds as rs2.
            // NOTE: Cannot use EXPECT_EQ with iterators.
            EXPECT_TRUE(it2 == rs2.end());
        }


        // Configure a TermTable with a set of Adhoc Terms and then verify
        // that RowIdSequence enumerates reasonable RowIds for each term.
        // Note that without knowledge of the hash function, we don't know
        // which RowIndex values to expect, but we do know they must be
        // unique for a given term and in range.
        TEST(TermTable, AdhocRows)
        {
            //
            // Configure TermTable.
            //

            TermTable termTable;

            const size_t maxRowsPerConfiguration = 5;
            const size_t explicitRowCount = 75;

            std::vector<RowIndex> adhocRowCounts;
            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                // Choose a large minimum number of adhoc rows at each rank to
                // reduce the possibility of a RowIndex collision. Generating
                // RowIndexes for a small collection of rows is likely to cause
                // collisions.
                const size_t minAdhocRowsPerRank = 1000;
                adhocRowCounts.push_back(100 * rank + minAdhocRowsPerRank);
            }


            // recipes stores the RowId used to configure the TermTable for
            // each (IdfX10, GramSize) pair and later verify generated RowIDs.
            std::array<
                std::array<
                    std::vector<RowId>,
                    Term::c_maxGramSize + 1>,
                    Term::c_maxIdfX10Value + 1>
                recipes;

            // For each (IdfX10, GramSize) pair
            for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
            {
                for (Term::GramSize gramSize = 0; gramSize <= Term::c_maxGramSize; ++gramSize)\
                {
                    // Invent a number of rows for this (IdfX10, GramSize) pair
                    size_t rowCount = (idf + gramSize) % maxRowsPerConfiguration;

                    // Create term entry.
                    termTable.OpenTerm();

                    for (size_t row = 0; row < rowCount; ++row)
                    {
                        // Invent a rank for this row. In this case just use
                        // the row number for the rank.
                        Rank rank = row;
                        RowId rowId(rank, 0);

                        // Record this RowId in the recipes for verification
                        recipes[idf][gramSize].push_back(rowId);

                        // Add this RowId to the current Term's configuration.
                        termTable.AddRowId(rowId);
                    }

                    termTable.CloseAdhocTerm(idf, gramSize);
                }
            }

            // Configure row counts in TermTable. The number of adhoc rows will
            // be necessary to verify that generated RowIndex values are in
            // range.
            for (Rank rank = 0; rank < c_maxRankValue; ++rank)
            {
                termTable.SetRowCounts(rank, explicitRowCount, adhocRowCounts[rank]);
            }
            termTable.SetFactCount(0);
            termTable.Seal();


            //
            // Verify rows for adhoc terms.
            //

            // Start with an arbitrary term hash well above the reserved hash
            // values for system rows and facts.
            Term::Hash hash = 12345ull;

            // For each (IdfX10, GramSize) pair
            for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
            {
                for (Term::GramSize gramSize = 0; gramSize <= Term::c_maxGramSize; ++gramSize)\
                {
                    // Generate a term.
                    Term term(hash++, 0, idf, gramSize);

                    std::vector<RowId> const & recipe = recipes[idf][gramSize];

                    // Verify that the RowIdSequence has the correct number of
                    // rows with the correct Ranks and unique RowIndex values
                    // that are in range.
                    VerifyAdhocRowIds(term, termTable, recipe, adhocRowCounts);

                    // Verify that two different terms at (idf, gramSize) get
                    // similar row patterns with different RowIndex values.
                    VerifyTwoTerms(term, termTable);
                }
            }

            // Verify roundtrip
            // Normally one wants to do black box testing, but it is hard to
            // devise a solid black box test because it is not possible for a
            // test to enumerate all of the terms in the TermTable and all of
            // the adhoc term recipes.
            std::stringstream stream;
            termTable.Write(stream);

            TermTable termTable2(stream);

            EXPECT_EQ(termTable, termTable2);
        }


        //*********************************************************************
        //
        // Test system rows.
        //
        //*********************************************************************

        TEST(TermTable, SystemRows)
        {
            TermTable termTable;
            termTable.AddRowId(RowId(0, 0));     // TermTable needs at least one row to function.

            // Configurate row counts.
            const size_t explicitRowCount = 100;
            const size_t adhocRowCount = 200;
            const size_t factCount = 7;
            // System rows and additional facts live at the end of the explicitRow space and are explicit rows.
            const size_t systemRowStart = adhocRowCount + explicitRowCount - factCount - 3;
            termTable.SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable.SetFactCount(factCount);
            termTable.Seal();


            // Soft Deleted Term
            {
                RowIdSequence rows(termTable.GetDocumentActiveTerm(), termTable);
                auto it = rows.begin();
                EXPECT_EQ(*it, RowId(0, systemRowStart + ITermTable::SystemTerm::DocumentActive));

                // Verify that iterator contains a single row. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                ++it;
                EXPECT_TRUE(it == rows.end());
            }

            {
                // Match All Term
                RowIdSequence rows(termTable.GetMatchAllTerm(), termTable);
                auto it = rows.begin();
                EXPECT_EQ(*it, RowId(0, systemRowStart + ITermTable::SystemTerm::MatchAll));

                // Verify that iterator contains a single row. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                ++it;
                EXPECT_TRUE(it == rows.end());
            }

            {
                // Soft Deleted Term
                RowIdSequence rows(termTable.GetMatchNoneTerm(), termTable);
                auto it = rows.begin();
                EXPECT_EQ(*it, RowId(0, systemRowStart + ITermTable::SystemTerm::MatchNone));

                // Verify that iterator contains a single row. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                ++it;
                EXPECT_TRUE(it == rows.end());
            }
        }


        //*********************************************************************
        //
        // Test ranks in use.
        //
        //*********************************************************************

        TEST(TermTable, RanksInUse)
        {
            std::array<bool, c_maxRankValue + 1> ranksInUse{};
            ranksInUse[0] = true;
            ranksInUse[4] = true;

            TermTable termTable;
            termTable.OpenTerm();
            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                if (ranksInUse[rank])
                {
                    termTable.AddRowId(RowId(rank, 0));
                }
            }
            termTable.CloseTerm(0ull);
            termTable.Seal();

            EXPECT_EQ(termTable.GetMaxRankUsed(), 4u);

            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                EXPECT_EQ(ranksInUse[rank], termTable.IsRankUsed(rank));
            }
        }


        //*********************************************************************
        //
        // Test fact rows.
        //
        //*********************************************************************

        TEST(TermTable, FactRows)
        {
            // TODO: Implement this test.
        }


        //*********************************************************************
        //
        // Test TermTable assertions.
        //
        //*********************************************************************

        TEST(TermTable, ThrowOnSealBeforeSet)
        {
            // TODO: Implement this test.
            // SetRowCounts for all ranks.
            // SetFactCount
            // Seal()
        }


        //*********************************************************************
        //
        // Test TermTable persistance to stream.
        //
        //*********************************************************************

        TEST(TermTable, RoundTrip)
        {
            // TODO: Implement this test.
        }
    }
}
