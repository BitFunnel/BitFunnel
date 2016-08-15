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

#include "BitFunnel/RowIdSequence.h"
#include "TermTable.h"


namespace BitFunnel
{
    namespace TableTest
    {
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
                    termTable.AddRowId(RowId(0, 0, r));
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
                    RowId expected = RowId(0, 0, r + adhocRowCount);
                    RowId observed = *it;
                    EXPECT_EQ(expected, observed);
                }

                // Verify that iterator is now at the end. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                EXPECT_TRUE(it == rows.end());
            }
        }


        TEST(TermTable, AdhocRows)
        {
            // Row counts correct
            // Ranks correct
            // RowIndexes all different
            // RowIndexes all in range 0..adhocRowCount
            // Terms with different hashes but same treatments get different rows.
        }


        TEST(TermTable, SystemRows)
        {
            TermTable termTable;
            termTable.AddRowId(RowId(0, 0, 0));     // TermTable needs at least one row to function.

            // Configurate row counts.
            const size_t explicitRowCount = 100;
            const size_t adhocRowCount = 200;
            const size_t systemRowStart = explicitRowCount + adhocRowCount;
            termTable.SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable.SetFactCount(7);
            termTable.Seal();


            // Soft Deleted Term
            {
                RowIdSequence rows(termTable.GetSoftDeletedTerm(), termTable);
                auto it = rows.begin();
                RowId observed = *it;
                EXPECT_EQ(*it, RowId(0, 0, systemRowStart + ITermTable2::SystemTerm::SoftDeleted));

                // Verify that iterator contains a single row. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                ++it;
                EXPECT_TRUE(it == rows.end());
            }

            {
                // Match All Term
                RowIdSequence rows(termTable.GetMatchAllTerm(), termTable);
                auto it = rows.begin();
                RowId observed = *it;
                EXPECT_EQ(*it, RowId(0, 0, systemRowStart + ITermTable2::SystemTerm::MatchAll));

                // Verify that iterator contains a single row. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                ++it;
                EXPECT_TRUE(it == rows.end());
            }

            {
                // Soft Deleted Term
                RowIdSequence rows(termTable.GetMatchNoneTerm(), termTable);
                auto it = rows.begin();
                RowId observed = *it;
                EXPECT_EQ(*it, RowId(0, 0, systemRowStart + ITermTable2::SystemTerm::MatchNone));

                // Verify that iterator contains a single row. NOTE: Cannot use
                // EXPECT_EQ with iterators.
                ++it;
                EXPECT_TRUE(it == rows.end());
            }
        }


        TEST(TermTable, ThrowOnSealBeforeSet)
        {
            // SetRowCounts for all ranks.
            // SetFactCount
            // Seal()
        }


        TEST(TermTable, RoundTrip)
        {
        }
    }
}
