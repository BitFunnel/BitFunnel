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

// For std::equal() on Windows.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include <iostream>         // TODO: Remove this temporary include.
#include <sstream>

#include "gtest/gtest.h"

#include "BitFunnel/RowIdSequence.h"
#include "DocumentFrequencyTable.h"
#include "TermTable.h"
#include "TermTableBuilder.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    class MockTermTreatment : public ITermTreatment
    {
    public:

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override
        {
            return m_configurations[term.GetRawHash()];
        }

        void OpenConfiguration()
        {
            m_current = RowConfiguration();
        }

        void AddEntry(Rank rank, RowIndex count, bool isPrivate)
        {
            m_current.push_front(RowConfiguration::Entry(rank, count, isPrivate));
        }

        void CloseConfiguration()
        {
            m_configurations.push_back(m_current);
        }

    private:
        RowConfiguration m_current;

        // MockTermTreatment uses the Term::Hash as the index into the vector
        // of RowConfigurations.
        std::vector<RowConfiguration> m_configurations;
    };

    class TestEnvironment
    {
    public:
        TestEnvironment()
        {
            // For this test, configurations will be indexed by the Term::Hash
            // values which we expect will be presented to the TermTableBuilder
            // in increasing order from 0 to ??.

            // Therefore we add RowConfigurations() in the order that we expect
            // they will be used by the TermTableBuilder. Each RowConfiguration
            // will be used once.

            //
            // Construct DocumentFrequencyTable
            //
            std::stringstream input;
            input <<                  // Rank:RowIndex
                "0,1,1,.9\n"          // 0:0
                "1,1,1,.7\n"          // 0:1
                "2,1,1,.07\n"         // 0:2
                "3,1,1,.05\n"         // 0:3
                "4,1,1,.02\n"         // 0:2
                "5,1,1,.0099\n"       // 0:2, 0:3
                "6,1,1,.0098\n"       // 0:3, 4:0
                "";

            m_documentFrequencyTable.reset(new DocumentFrequencyTable(input));

            //
            // Construct TermTreatment data.
            //

            //
            // Construct expected TermTable.
            //
            std::vector<RowIndex> rows;
            for (Rank r = 0; r <= c_maxRankValue; ++r)
            {
                rows.push_back(0);
            }

            Term::Hash hash = 0ull;

            // First term configured as private rank 0 so it should go in its
            // own row.
            m_termTreatment.OpenConfiguration();
            m_termTreatment.AddEntry(0, 1, true);
            m_termTreatment.CloseConfiguration();

            m_termTable.OpenTerm();
            m_termTable.AddRowId(RowId(0, 0, rows[0]++));
            m_termTable.CloseTerm(hash++);

            // Second term is configurated as a single shared rank 0 row, but
            // will be placed in its own row because of its high density of 0.7.
            m_termTreatment.OpenConfiguration();
            m_termTreatment.AddEntry(0, 1, false);
            m_termTreatment.CloseConfiguration();

            m_termTable.OpenTerm();
            m_termTable.AddRowId(RowId(0, 0, rows[0]++));
            m_termTable.CloseTerm(hash++);

            // Third row is configured as a single shared rank 0 row. It's
            // density of 0.07 is low enough that it will share with the fifth
            // row.
            m_termTreatment.OpenConfiguration();
            m_termTreatment.AddEntry(0, 1, false);
            m_termTreatment.CloseConfiguration();

            RowIndex third = rows[0];
            m_termTable.OpenTerm();
            m_termTable.AddRowId(RowId(0, 0, rows[0]++));
            m_termTable.CloseTerm(hash++);

            // Fourth row is configured as a single shared rank 0 row. It's
            // density of 0.05 is low enough that it will share with the sixth
            // and seventh rows.
            m_termTreatment.OpenConfiguration();
            m_termTreatment.AddEntry(0, 1, false);
            m_termTreatment.CloseConfiguration();

            RowIndex fourth = rows[0];
            m_termTable.OpenTerm();
            m_termTable.AddRowId(RowId(0, 0, rows[0]++));
            m_termTable.CloseTerm(hash++);

            // Fifth row is configured as a single shared rank 0 row. It's
            // density of 0.02 is low enough that it will share with the
            // third row.
            m_termTreatment.OpenConfiguration();
            m_termTreatment.AddEntry(0, 1, false);
            m_termTreatment.CloseConfiguration();

            m_termTable.OpenTerm();
            m_termTable.AddRowId(RowId(0, 0, third));
            m_termTable.CloseTerm(hash++);

            // Sixth row is configured as a pair of shared rank 0 rows. It's
            // density of 0.01 is low enough that it will share with the third
            // and fourth rows.
            m_termTreatment.OpenConfiguration();
            m_termTreatment.AddEntry(0, 2, false);
            m_termTreatment.CloseConfiguration();

            m_termTable.OpenTerm();
            m_termTable.AddRowId(RowId(0, 0, third));
            m_termTable.AddRowId(RowId(0, 0, fourth));
            m_termTable.CloseTerm(hash++);

            // Seventh row is configured two shared rows, one rank 0 and one
            // rank 4. Seventh row's frequency of 0.01 is great enough to 
            // require a private row at rank 4.
            m_termTreatment.OpenConfiguration();
            m_termTreatment.AddEntry(4, 1, false);
            m_termTreatment.AddEntry(0, 1, false);
            m_termTreatment.CloseConfiguration();

            m_termTable.OpenTerm();
            m_termTable.AddRowId(RowId(0, 0, fourth));
            m_termTable.AddRowId(RowId(0, 4, rows[4]++));
            m_termTable.CloseTerm(hash++);
        }


        DocumentFrequencyTable const & GetDocFrequencyTable() const
        {
            return *m_documentFrequencyTable;
        }


        TermTable const & GetTermTable() const
        {
            return m_termTable;
        }


        ITermTreatment const & GetTermTreatment() const
        {
            return m_termTreatment;
        }

private:
        std::unique_ptr<DocumentFrequencyTable> m_documentFrequencyTable;
        MockTermTreatment m_termTreatment;
        TermTable m_termTable;
    };


    namespace TermTableBuilderTest
    {
        TEST(TermTableBuilder, GeneralCases)
        {
            // Construct a test environment that provides the ITermTreatment
            // and DocumentFrequencyTable to be used by the TermTableBuilder.
            // The test environement also provides a hand-constructed expected
            // TermTable for verification purposes.
            TestEnvironment environment;

            // Run the TermTableBuilder to configure a TermTable.
            ITermTreatment const & treatment = environment.GetTermTreatment();
            DocumentFrequencyTable const & terms = environment.GetDocFrequencyTable();
            TermTable termTable;
            double density = 0.1;
            double adhocFrequency = 0.0001;
            TermTableBuilder builder(density, adhocFrequency, treatment, terms, termTable);

            //
            // Verify that configured TermTable is the same as the expected
            // TermTable provided by the enviornment.
            //

            // Ensure that all known terms have the same RowIdSequences.
            // NOTE: This test relies on the existence of a separate unit test
            // for TermTable that ensures that RowIds and Terms are added
            // correctly. Without such a test, a bogus TermTable that ignores
            // all RowId additions would allow TermTableBuilderTest to pass.
            for (auto term : terms)
            {
                RowIdSequence expected(term.GetTerm(), environment.GetTermTable());
                RowIdSequence observed(term.GetTerm(), termTable);
                EXPECT_TRUE(std::equal(observed.begin(), observed.end(), expected.begin()));
                EXPECT_TRUE(std::equal(expected.begin(), expected.end(), observed.begin()));
            }


            // NOTE: This test relies on the existence of a separate unit test
            // for TermTable that ensures that row counts are recorded
            // correctly. Without such a test, a bogus TermTable that ignores
            // all SetRowCounts would allow TermTableBuilderTest to pass.
            for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
            {
                EXPECT_EQ(termTable.GetTotalRowCount(rank), environment.GetTermTable().GetTotalRowCount(rank));
            }

            // TODO: Verify adhoc
            // TODO: Verify facts
            // TODO: Verify row counts.
        }
    }
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
