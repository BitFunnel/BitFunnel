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

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/ITermTableCollection.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/TextObjectFormatter.h"
#include "RowMatchNode.h"
#include "TextObjectParser.h"
#include "TermPlanConverter.h"
// #include "BitFunnel/FalsePositiveEvaluationNode.h"
// #include "MockIndexConfiguration.h"
// #include "MockTermTable.h"


namespace BitFunnel
{
    // Duplicated code from Term.cpp. TODO: figure out how we should handle
    // this. This is here because creating an N-gram from a Term requires a
    // configuration, and using this was simpler.
    static uint64_t rotl64By1(uint64_t x)
    {
        return (x << 1) | (x >> 63);
    }


    namespace TermPlanConverterUnitTest
    {
        void VerifyTermPlanConverterCase(char const * inputTermPlan,
                                         char const * expectedRowPlan,
                                         ISimpleIndex const & index)
        {
            // TODO: this can probably be smaller.
            Allocator allocator(4096*256);

            std::stringstream input(inputTermPlan);
            TextObjectParser parser(input, allocator, &TermMatchNode::GetType);

            TermMatchNode const & termMatchNode = TermMatchNode::Parse(parser);

            // Verify RowPlan.
            const RowPlan& rowPlan = TermPlanConverter::BuildRowPlan(termMatchNode,
                                                                     index,
                                                                     allocator);

            std::stringstream outputForRowPlan;
            TextObjectFormatter formatterForRowPlan(outputForRowPlan);
            rowPlan.Format(formatterForRowPlan);

            std::string expected(expectedRowPlan);
            EXPECT_EQ(expected, outputForRowPlan.str());

        }


        static std::vector<DocIndex> CreateDefaultShardCapacities()
        {
            const std::vector<DocIndex> capacities = { 4096 };
            return capacities;
        }

        static std::vector<DocIndex> s_defaultShardCapacities(CreateDefaultShardCapacities());


        TEST(TermPlanConverter,Unigram)
        {
            auto filesystem = Factories::CreateFileSystem();
            auto index = Factories::CreateSimpleIndex(*filesystem);

            auto termTable = Factories::CreateTermTable();
            const size_t adhocRowCount = 1;
            auto hash = Term::ComputeRawHash("foo");

            termTable->OpenTerm();
            RowIndex explicitRowCount = ITermTable::SystemTerm::Count;
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            termTable->SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable->Seal();

            auto termTableCollection = Factories::CreateTermTableCollection();
            termTableCollection->AddTermTable(std::move(termTable));

            index->SetTermTableCollection(std::move(termTableCollection));
            index->ConfigureAsMock(1, false);
            index->StartIndex();

            // 13 is the stream.  TODO: figure out what stream it should be when
            // we have real StreamId support.
            char const * input = "Unigram(\"foo\", 13)";

            char const * expectedFullQueryPlan =
                "RowPlan {\n"
                "  Match: And {\n"
                "    Children: [\n"
                "      Row(2, 0, 0, false),\n"
                "      Row(1, 0, 0, false),\n"

                // Soft-deleted row.
                "      Row(0, 0, 0, false)\n"
                "    ]\n"
                "  }\n"
                "}";

            // Generate full query plan.
            VerifyTermPlanConverterCase(input,
                                        expectedFullQueryPlan,
                                        *index);
        }


        TEST(TermPlanConverter,And)
        {
            // MockIndexConfiguration index(s_defaultShardCapacities);
            auto filesystem = Factories::CreateFileSystem();
            auto index = Factories::CreateSimpleIndex(*filesystem);

            auto termTable = Factories::CreateTermTable();
            const size_t adhocRowCount = 4;

            termTable->OpenTerm();
            auto hash = Term::ComputeRawHash("foo");
            RowIndex explicitRowCount = ITermTable::SystemTerm::Count;
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            // Inserting a dummy term doesn't change the Row numbers in the
            // expected result. This is because the expected result is a
            // RowPlan, which consists of AbstractRows. AbstractRows have an
            // index that's relative to the plan (they index into a table of
            // RowIds used in the plan); those aren't "physical" row addresses.
            hash = Term::ComputeRawHash("dummyTerm");
            termTable->OpenTerm();
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            termTable->OpenTerm();
            hash = Term::ComputeRawHash("bar");
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            termTable->SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable->Seal();

            auto termTableCollection = Factories::CreateTermTableCollection();
            termTableCollection->AddTermTable(std::move(termTable));

            index->SetTermTableCollection(std::move(termTableCollection));
            index->ConfigureAsMock(1, false);
            index->StartIndex();

            // 13 is the stream.  TODO: figure out what stream it should be when
            // we have real StreamId support.
            char const * input =
                "And {\n"
                "  Children: [\n"
                "    Unigram(\"bar\", 13),\n"
                "    Unigram(\"foo\", 13)\n"
                "  ]\n"
                "}";


            char const * expectedFullQueryPlan =
                "RowPlan {\n"
                "  Match: And {\n"
                "    Children: [\n"
                "      Row(4, 0, 0, false),\n"
                "      Row(3, 0, 0, false),\n"
                "      Row(2, 0, 0, false),\n"
                "      Row(1, 0, 0, false),\n"

                // Soft-deleted row.
                "      Row(0, 0, 0, false)\n"
                "    ]\n"
                "  }\n"
                "}";

            // Generate full query plan.
            VerifyTermPlanConverterCase(input,
                                        expectedFullQueryPlan,
                                        *index);
        }


        TEST(TermPlanConverter,Or)
        {
            // MockIndexConfiguration index(s_defaultShardCapacities);
            auto filesystem = Factories::CreateFileSystem();
            auto index = Factories::CreateSimpleIndex(*filesystem);

            auto termTable = Factories::CreateTermTable();
            const size_t adhocRowCount = 4;

            termTable->OpenTerm();
            auto hash = Term::ComputeRawHash("foo");
            RowIndex explicitRowCount = ITermTable::SystemTerm::Count;
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            termTable->OpenTerm();
            hash = Term::ComputeRawHash("bar");
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            termTable->SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable->Seal();

            auto termTableCollection = Factories::CreateTermTableCollection();
            termTableCollection->AddTermTable(std::move(termTable));

            index->SetTermTableCollection(std::move(termTableCollection));
            index->ConfigureAsMock(1, false);
            index->StartIndex();

            // 13 is the stream.  TODO: figure out what stream it should be when
            // we have real StreamId support.
            char const * input =
                "Or {\n"
                "  Children: [\n"
                "    Unigram(\"bar\", 13),\n"
                "    Unigram(\"foo\", 13)\n"
                "  ]\n"
                "}";


            char const * expectedFullQueryPlan =
                "RowPlan {\n"
                "  Match: And {\n"
                "    Children: [\n"
                "      Or {\n"
                "        Children: [\n"
                "          And {\n"
                "            Children: [\n"
                "              Row(4, 0, 0, false),\n"
                "              Row(3, 0, 0, false)\n"
                "            ]\n"
                "          },\n"
                "          And {\n"
                "            Children: [\n"
                "              Row(2, 0, 0, false),\n"
                "              Row(1, 0, 0, false)\n"
                "            ]\n"
                "          }\n"
                "        ]\n"
                "      },\n"
                "      Row(0, 0, 0, false)\n"
                "    ]\n"
                "  }\n"
                "}";

            // Generate full query plan.
            VerifyTermPlanConverterCase(input,
                                        expectedFullQueryPlan,
                                        *index);
        }


        TEST(TermPlanConverter,AndNot)
        {
            // MockIndexConfiguration index(s_defaultShardCapacities);
            auto filesystem = Factories::CreateFileSystem();
            auto index = Factories::CreateSimpleIndex(*filesystem);

            auto termTable = Factories::CreateTermTable();
            const size_t adhocRowCount = 4;

            termTable->OpenTerm();
            auto hash = Term::ComputeRawHash("foo");
            RowIndex explicitRowCount = ITermTable::SystemTerm::Count;
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            termTable->OpenTerm();
            hash = Term::ComputeRawHash("bar");
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(hash);

            termTable->SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable->Seal();

            auto termTableCollection = Factories::CreateTermTableCollection();
            termTableCollection->AddTermTable(std::move(termTable));

            index->SetTermTableCollection(std::move(termTableCollection));
            index->ConfigureAsMock(1, false);
            index->StartIndex();


            // 13 is the stream.  TODO: figure out what stream it should be when
            // we have real StreamId support.
            char const * input =
                "And {\n"
                "  Children: [\n"
                "    Unigram(\"bar\", 13),\n"
                "    Not {\n"
                "      Child: Unigram(\"foo\", 13)\n"
                "    }\n"
                "  ]\n"
                "}";


            char const * expectedFullQueryPlan =
                "RowPlan {\n"
                "  Match: And {\n"
                "    Children: [\n"
                "      Not {\n"
                "        Child: And {\n"
                "          Children: [\n"
                "            Row(4, 0, 0, false),\n"
                "            Row(3, 0, 0, false)\n"
                "          ]\n"
                "        }\n"
                "      },\n"
                "      Row(2, 0, 0, false),\n"
                "      Row(1, 0, 0, false),\n"

                // Soft-deleted row.
                "      Row(0, 0, 0, false)\n"
                "    ]\n"
                "  }\n"
                "}";

            // Generate full query plan.
            VerifyTermPlanConverterCase(input,
                                        expectedFullQueryPlan,
                                        *index);
        }


        TEST(TermPlanConverter,Phrase)
        {
            // MockIndexConfiguration index(s_defaultShardCapacities);
            auto filesystem = Factories::CreateFileSystem();
            auto index = Factories::CreateSimpleIndex(*filesystem);

            auto termTable = Factories::CreateTermTable();
            const size_t adhocRowCount = 4;

            termTable->OpenTerm();
            auto fooHash = Term::ComputeRawHash("foo");
            RowIndex explicitRowCount = ITermTable::SystemTerm::Count;
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(fooHash);

            termTable->OpenTerm();
            auto barHash = Term::ComputeRawHash("bar");
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(barHash);

            auto bazHash = Term::ComputeRawHash("baz");
            termTable->OpenTerm();
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(bazHash);

            // Inserting a dummy term doesn't change the Row numbers in the
            // expected result. This is because the expected result is a
            // RowPlan, which consists of AbstractRows. AbstractRows have an
            // index that's relative to the plan (they index into a table of
            // RowIds used in the plan); those aren't "physical" row addresses.
            auto dummyHash = Term::ComputeRawHash("dummyTerm");
            termTable->OpenTerm();
            while (explicitRowCount < 16)
            {
                termTable->AddRowId(RowId(0, explicitRowCount++));
            }
            termTable->CloseTerm(dummyHash);

            // "foo bar".
            auto fooBarHash = rotl64By1(fooHash) ^ barHash;
            termTable->OpenTerm();
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(fooBarHash);

            // "bar baz".
            auto barBazHash = rotl64By1(barHash) ^ bazHash;
            termTable->OpenTerm();
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(barBazHash);

            // "foo bar baz".
            auto fooBarBazHash = rotl64By1(fooBarHash) ^ bazHash;
            termTable->OpenTerm();
            termTable->AddRowId(RowId(0, explicitRowCount++));
            termTable->CloseTerm(fooBarBazHash);


            termTable->SetRowCounts(0, explicitRowCount, adhocRowCount);
            termTable->Seal();

            auto termTableCollection = Factories::CreateTermTableCollection();
            termTableCollection->AddTermTable(std::move(termTable));

            index->SetTermTableCollection(std::move(termTableCollection));
            index->ConfigureAsMock(3, false);
            index->StartIndex();

            // 13 is the stream.  TODO: figure out what stream it should be when
            // we have real StreamId support.
            char const * input =
                "Phrase {\n"
                "  StreamId: 13,\n"
                "  Grams: [\n"
                "    \"foo\",\n"
                "    \"bar\",\n"
                "    \"baz\"\n"
                "  ]\n"
                "}";

            char const * expectedFullQueryPlan =
                "RowPlan {\n"
                "  Match: And {\n"
                "    Children: [\n"
                // One row per sub-phrase.
                "      Row(12, 0, 0, false),\n"
                "      Row(11, 0, 0, false),\n"
                "      Row(10, 0, 0, false),\n"
                // baz
                "      Row(9, 0, 0, false),\n"
                "      Row(8, 0, 0, false),\n"
                "      Row(7, 0, 0, false),\n"
                "      Row(6, 0, 0, false),\n"
                // bar
                "      Row(5, 0, 0, false),\n"
                "      Row(4, 0, 0, false),\n"
                "      Row(3, 0, 0, false),\n"
                // foo
                "      Row(2, 0, 0, false),\n"
                "      Row(1, 0, 0, false),\n"

                // Soft-deleted row.
                "      Row(0, 0, 0, false)\n"
                "    ]\n"
                "  }\n"
                "}";

            // Generate full query plan.
            VerifyTermPlanConverterCase(input,
                                        expectedFullQueryPlan,
                                        *index);
        }


        // TODO: need at least one test that tests ad hoc rows.

        // TODO: need to implement nonBody.
        // TEST(TermPlanConverter,NonBodyPlanWithFull)
        // {
        //     MockIndexConfiguration index(s_defaultShardCapacities);

        //     char const * input = "Unigram(\"foo\", full)";

        //     char const * expectedNonBodyQueryPlan =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // foo @ nonbody (2, 4, 2)
        //         "      Row(9, 0, 0, false),\n"
        //         "      Row(8, 0, 0, false),\n"
        //         "      Row(13, 3, 0, false),\n"
        //         "      Row(12, 3, 0, false),\n"
        //         "      Row(11, 3, 0, false),\n"
        //         "      Row(10, 3, 0, false),\n"
        //         "      Row(15, 6, 0, false),\n"
        //         "      Row(14, 6, 0, false),\n"

        //         // foo @ full (2, 4, 1)
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(1, 0, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(3, 3, 0, false),\n"
        //         "      Row(7, 6, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* grams[] = { "foo", nullptr };
        //     Stream::Classification classification = Stream::Full;

        //     expectedFalsePositiveEvaluationPlan
        //         << "Term {\n"
        //         << "  Children: FPMatchData(" << GetClassifiedHash(grams, nullptr, classification) << ", 1)\n"
        //         << "}";

        //     // Generate nonbody query plan.
        //     VerifyTermPlanConverterCase(input,
        //                                 expectedNonBodyQueryPlan,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 true);
        // }

        // TODO: need to implement nonBody and MetaWord.
        // TEST(TermPlanConverter,NonBodyPlanWithMetaword)
        // {
        //     MockIndexConfiguration index(s_defaultShardCapacities);

        //     char const * input = "Unigram(\"foo\", metaword)";

        //     char const * expectedMetawordQueryPlan =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // foo @ metaword (2, 4, 1)
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(1, 0, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(3, 3, 0, false),\n"
        //         "      Row(7, 6, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* grams[] = { "foo", nullptr };
        //     Stream::Classification classification = Stream::MetaWord;

        //     expectedFalsePositiveEvaluationPlan
        //         << "Term {\n"
        //         << "  Children: FPMatchData(" << GetClassifiedHash(grams, nullptr, classification) << ", 1)\n"
        //         << "}";

        //     // Verify full plan case.
        //     VerifyTermPlanConverterCase(input,
        //                                 expectedMetawordQueryPlan,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 false);


        //     // Verify nonbody plan case.
        //     // For row plan, the Non body version should be identical to metaword's.
        //     char const * expectedNonBodyQueryPlan = expectedMetawordQueryPlan;

        //     // Generate nonbody query plan.
        //     VerifyTermPlanConverterCase(input,
        //                                 expectedNonBodyQueryPlan,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 true);
        // }


        // TEST(TermPlanConverter,NonBodyPlanWithPhrase)
        // {
        //     MockIndexConfiguration index(s_defaultShardCapacities);

        //     char const * input =
        //         "Phrase {\n"
        //         "  Classification: full,\n"
        //         "  Suffix: nullable(), \n"
        //         "  Grams: [\n"
        //         "    \"123\",\n"
        //         "    \"456\",\n"
        //         "    \"789\"\n"
        //         "  ]\n"
        //         "}";

        //     char const * expectedNonBodyQueryPlan =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // 789 @ nonbody (1, 4, 0)
        //         "      Row(59, 0, 0, false),\n"
        //         "      Row(63, 3, 0, false),\n"
        //         "      Row(62, 3, 0, false),\n"
        //         "      Row(61, 3, 0, false),\n"
        //         "      Row(60, 3, 0, false),\n"

        //         // 789 @ full (1, 3, 1)
        //         "      Row(54, 0, 0, false),\n"
        //         "      Row(57, 3, 0, false),\n"
        //         "      Row(56, 3, 0, false),\n"
        //         "      Row(55, 3, 0, false),\n"
        //         "      Row(58, 6, 0, false),\n"

        //         // 456,789 @ nonbody (2, 4, 1)
        //         "      Row(48, 0, 0, false),\n"
        //         "      Row(47, 0, 0, false),\n"
        //         "      Row(52, 3, 0, false),\n"
        //         "      Row(51, 3, 0, false),\n"
        //         "      Row(50, 3, 0, false),\n"
        //         "      Row(49, 3, 0, false),\n"
        //         "      Row(53, 6, 0, false),\n"

        //         // 456,789 @ full (1, 1, 2)
        //         "      Row(43, 0, 0, false),\n"
        //         "      Row(44, 3, 0, false),\n"
        //         "      Row(46, 6, 0, false),\n"
        //         "      Row(45, 6, 0, false),\n"

        //         // 456 @ nonbody (2, 0, 0)
        //         "      Row(42, 0, 0, false),\n"
        //         "      Row(41, 0, 0, false),\n"

        //         // 456 @ full (1, 2, 1)
        //         "      Row(37, 0, 0, false),\n"
        //         "      Row(39, 3, 0, false),\n"
        //         "      Row(38, 3, 0, false),\n"
        //         "      Row(40, 6, 0, false),\n"

        //         // 123,456,789 @ nonbody (1, 1, 4)
        //         "      Row(31, 0, 0, false),\n"
        //         "      Row(32, 3, 0, false),\n"
        //         "      Row(36, 6, 0, false),\n"
        //         "      Row(35, 6, 0, false),\n"
        //         "      Row(34, 6, 0, false),\n"
        //         "      Row(33, 6, 0, false),\n"

        //         // 123,456,789 @ full (2, 0, 0)
        //         "      Row(30, 0, 0, false),\n"
        //         "      Row(29, 0, 0, false),\n"

        //         // 123,456 @ nonbody (1, 3, 4)
        //         "      Row(21, 0, 0, false),\n"
        //         "      Row(24, 3, 0, false),\n"
        //         "      Row(23, 3, 0, false),\n"
        //         "      Row(22, 3, 0, false),\n"
        //         "      Row(28, 6, 0, false),\n"
        //         "      Row(27, 6, 0, false),\n"
        //         "      Row(26, 6, 0, false),\n"
        //         "      Row(25, 6, 0, false),\n"

        //         // 123,456 @ full (1, 3, 1)
        //         "      Row(16, 0, 0, false),\n"
        //         "      Row(19, 3, 0, false),\n"
        //         "      Row(18, 3, 0, false),\n"
        //         "      Row(17, 3, 0, false),\n"
        //         "      Row(20, 6, 0, false),\n"

        //         // 123 @ nonbody (2, 4, 2)
        //         "      Row(9, 0, 0, false),\n"
        //         "      Row(8, 0, 0, false),\n"
        //         "      Row(13, 3, 0, false),\n"
        //         "      Row(12, 3, 0, false),\n"
        //         "      Row(11, 3, 0, false),\n"
        //         "      Row(10, 3, 0, false),\n"
        //         "      Row(15, 6, 0, false),\n"
        //         "      Row(14, 6, 0, false),\n"

        //         // 123 @ full (2, 4, 1)
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(1, 0, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(3, 3, 0, false),\n"
        //         "      Row(7, 6, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     // Generate nonbody query plan.

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* wholePhrase[] = { "123", "456", "789", nullptr };
        //     char* subPhrase1[] = { "123", nullptr };
        //     char* subPhrase2[] = { "123", "456", nullptr };
        //     char* subPhrase3[] = { "456", nullptr };
        //     char* subPhrase4[] = { "456", "789", nullptr };
        //     char* subPhrase5[] = { "789", nullptr };
        //     Stream::Classification classification = Stream::Full;

        //     expectedFalsePositiveEvaluationPlan
        //         << "Phrase {\n"
        //         << "  Base: FPMatchData(" << GetClassifiedHash(wholePhrase, nullptr, classification) << ", 3),\n"
        //         << "  Subphrases: [\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase1, nullptr, classification) << ", 1),\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase2, nullptr, classification) << ", 2),\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase3, nullptr, classification) << ", 1),\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase4, nullptr, classification) << ", 2),\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase5, nullptr, classification) << ", 1)\n"
        //         << "  ]\n"
        //         << "}";

        //     VerifyTermPlanConverterCase(input,
        //                                 expectedNonBodyQueryPlan,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 true);
        // }


        // void AddRowsForFact(FactHandle handle,
        //                     MockTermTable& termTable)
        // {
        //     const Term term(static_cast<Term::Hash>(handle),
        //                     Stream::MetaWord,
        //                     static_cast<IdfX10>(0),
        //                     DDRTier);

        //     termTable.AddPrivateRowTerm(term, 0);
        // }


        // TEST(TermPlanConverter,UnigramAndFact)
        // {
        //     MockIndexConfiguration index(s_defaultShardCapacities);

        //     // User-defined facts go after the system facts.
        //     static const FactHandle factHandle(c_systemRowCount);
        //     MockTermTable& termTable = const_cast<MockTermTable&>(static_cast<const MockTermTable&>(
        //         *index.GetTermTables().GetTermTable(0)));

        //     AddRowsForFact(factHandle, termTable);

        //     char const * input =
        //         "And {\n"
        //         "  Children: [\n"
        //         "    Fact(1),\n"
        //         "    Unigram(\"foo\", full)\n"
        //         "  ]\n"
        //         "}";

        //     char const * expectedRowPlan =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // foo @ full (2, 4, 1)
        //         "      Row(3, 0, 0, false),\n"
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(7, 3, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(8, 6, 0, false),\n"

        //         // Fact(1) (1, 0, 0)
        //         "      Row(1, 0, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     // Generate and verify the row plan and false positive evaluation plan.
        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* grams[] = { "foo", nullptr };
        //     Stream::Classification classification = Stream::Full;

        //     // Fact is not relevant to false positive evaluation.
        //     expectedFalsePositiveEvaluationPlan
        //         << "Term {\n"
        //         << "  Children: FPMatchData(" << GetClassifiedHash(grams, nullptr, classification) << ", 1)\n"
        //         << "}";

        //     VerifyTermPlanConverterCase(input,
        //                                 expectedRowPlan,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 false);
        // }


        // TEST(TermPlanConverter,UnigramAndFactWithNonBody)
        // {
        //     MockIndexConfiguration index(s_defaultShardCapacities);

        //     // User-defined facts go after the system facts.
        //     static const FactHandle factHandle(c_systemRowCount);
        //     MockTermTable& termTable = const_cast<MockTermTable&>(static_cast<const MockTermTable&>(
        //         *index.GetTermTables().GetTermTable(0)));
        //     AddRowsForFact(factHandle, termTable);

        //     char const * input =
        //         "And {\n"
        //         "  Children: [\n"
        //         "    Fact(1),\n"
        //         "    Unigram(\"foo\", full)\n"
        //         "  ]\n"
        //         "}";

        //     char const * expectedRowPlan =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // foo @ nonbody (2, 4, 2)
        //         "      Row(10, 0, 0, false),\n"
        //         "      Row(9, 0, 0, false),\n"
        //         "      Row(14, 3, 0, false),\n"
        //         "      Row(13, 3, 0, false),\n"
        //         "      Row(12, 3, 0, false),\n"
        //         "      Row(11, 3, 0, false),\n"
        //         "      Row(16, 6, 0, false),\n"
        //         "      Row(15, 6, 0, false),\n"

        //         // foo @ full (2, 4, 1)
        //         "      Row(3, 0, 0, false),\n"
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(7, 3, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(8, 6, 0, false),\n"

        //         // Fact(1) (1, 0, 0)
        //         "      Row(1, 0, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     // Generate and verify the row plan and false positive evaluation plan.
        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* grams[] = { "foo", nullptr };
        //     Stream::Classification classification = Stream::Full;

        //     // Fact is not relevant to false positive evaluation.
        //     expectedFalsePositiveEvaluationPlan
        //         << "Term {\n"
        //         << "  Children: FPMatchData(" << GetClassifiedHash(grams, nullptr, classification) << ", 1)\n"
        //         << "}";

        //     VerifyTermPlanConverterCase(input,
        //                                 expectedRowPlan,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 true);
        // }


        // // Verify that a term exists in the term table and all of its rows
        // // belong to a given tier.
        // void VerifyTier(Term term, ITermTable const & termTable)
        // {
        //     TermInfo termInfo(term, termTable);
        //     TestAssert(!termInfo.IsEmpty());
        // }


        // // Verify that a term with the given text exists in the term table and
        // // all of its rows belong to a given tier.
        // void VerifyTier(const char * termText, ITermTable const & termTable)
        // {
        //     VerifyTier(Term(Term::ComputeRawHash(termText), Stream::Full, 0),
        //                termTable);
        // }


        // TEST(TermPlanConverter,PhraseNodeConversion)
        // {
        //     char const * input =
        //         "Phrase {\n"
        //         "  Classification: full,\n"
        //         "  Suffix: nullable(), \n"
        //         "  Grams: [\n"
        //         "    \"321\",\n"
        //         "    \"654\"\n"
        //         "  ]\n"
        //         "}";

        //     char const * expected =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // 654 @ full (1, 3, 1)
        //         "      Row(16, 0, 0, false),\n"
        //         "      Row(19, 3, 0, false),\n"
        //         "      Row(18, 3, 0, false),\n"
        //         "      Row(17, 3, 0, false),\n"
        //         "      Row(20, 6, 0, false),\n"

        //         // 321,654 @ full (2, 4, 2)
        //         "      Row(9, 0, 0, false),\n"
        //         "      Row(8, 0, 0, false),\n"
        //         "      Row(13, 3, 0, false),\n"
        //         "      Row(12, 3, 0, false),\n"
        //         "      Row(11, 3, 0, false),\n"
        //         "      Row(10, 3, 0, false),\n"
        //         "      Row(15, 6, 0, false),\n"
        //         "      Row(14, 6, 0, false),\n"

        //         // 321 @ full (2, 4, 1)
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(1, 0, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(3, 3, 0, false),\n"
        //         "      Row(7, 6, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* wholePhrase[] = { "321", "654", nullptr };
        //     char* subPhrase1[] = { "321", nullptr };
        //     char* subPhrase2[] = { "654", nullptr };
        //     Stream::Classification classification = Stream::Full;

        //     expectedFalsePositiveEvaluationPlan
        //         << "Phrase {\n"
        //         << "  Base: FPMatchData(" << GetClassifiedHash(wholePhrase, nullptr, classification) << ", 2),\n"
        //         << "  Subphrases: [\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase1, nullptr, classification) << ", 1),\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase2, nullptr, classification) << ", 1)\n"
        //         << "  ]\n"
        //         << "}";

        //     MockIndexConfiguration index(s_defaultShardCapacities);
        //     VerifyTermPlanConverterCase(input,
        //                                 expected,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 false);

        //     ITermTable const & termTable = *index.GetTermTables().GetTermTable(0);

        //     VerifyTier("654", termTable);
        //     VerifyTier("321", termTable);

        //     NGramBuilder ngram;
        //     ngram.AddGram(Term::ComputeRawHash("321"), 0);
        //     ngram.AddGram(Term::ComputeRawHash("654"), 0);
        //     const Term phraseTerm(ngram, Stream::Full, SSDTier);

        //     VerifyTier(phraseTerm, termTable);
        // }


        // TEST(TermPlanConverter,PhraseNodeWithSuffixConversion)
        // {
        //     char const * input =
        //         "Phrase {\n"
        //         "  Classification: full,\n"
        //         "  Suffix: nullable(\"Stream\"), \n"
        //         "  Grams: [\n"
        //         "    \"321\",\n"
        //         "    \"654\"\n"
        //         "  ]\n"
        //         "}";

        //     char const * expected =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // 654 @ full (1, 3, 1)
        //         "      Row(16, 0, 0, false),\n"
        //         "      Row(19, 3, 0, false),\n"
        //         "      Row(18, 3, 0, false),\n"
        //         "      Row(17, 3, 0, false),\n"
        //         "      Row(20, 6, 0, false),\n"

        //         // 321,654 @ full (2, 4, 2)
        //         "      Row(9, 0, 0, false),\n"
        //         "      Row(8, 0, 0, false),\n"
        //         "      Row(13, 3, 0, false),\n"
        //         "      Row(12, 3, 0, false),\n"
        //         "      Row(11, 3, 0, false),\n"
        //         "      Row(10, 3, 0, false),\n"
        //         "      Row(15, 6, 0, false),\n"
        //         "      Row(14, 6, 0, false),\n"

        //         // 321 @ full (2, 4, 1)
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(1, 0, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(3, 3, 0, false),\n"
        //         "      Row(7, 6, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* wholePhrase[] = { "321", "654", nullptr };
        //     char* subPhrase1[] = { "321", nullptr };
        //     char* subPhrase2[] = { "654", nullptr };
        //     Stream::Classification classification = Stream::Full;

        //     expectedFalsePositiveEvaluationPlan
        //         << "Phrase {\n"
        //         << "  Base: FPMatchData(" << GetClassifiedHash(wholePhrase, "Stream", classification) << ", 2),\n"
        //         << "  Subphrases: [\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase1, "Stream", classification) << ", 1),\n"
        //         << "    FPMatchData(" << GetClassifiedHash(subPhrase2, "Stream", classification) << ", 1)\n"
        //         << "  ]\n"
        //         << "}";

        //     MockIndexConfiguration index(s_defaultShardCapacities);
        //     VerifyTermPlanConverterCase(input,
        //         expected,
        //         expectedFalsePositiveEvaluationPlan.str().c_str(),
        //         index,
        //         false);

        //     ITermTable const & termTable = *index.GetTermTables().GetTermTable(0);

        //     VerifyTier("654", termTable);
        //     VerifyTier("321", termTable);

        //     NGramBuilder ngram;
        //     ngram.AddGram(Term::ComputeRawHash("321"), 0);
        //     ngram.AddGram(Term::ComputeRawHash("654"), 0);
        //     const Term phraseTerm(ngram, Stream::Full, SSDTier);

        //     VerifyTier(phraseTerm, termTable);
        // }


        // TEST(TermPlanConverter,SimpleTreeWithInteriorNodesConversion)
        // {
        //     char const * input =
        //         "And {\n"
        //         "  Children: [\n"
        //         "    Unigram(\"foo\", full),\n"
        //         "    Unigram(\"foobar\", metaword)\n"
        //         "  ]\n"
        //         "}";

        //     char const * expected =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // foobar @ metaword (2, 4, 2)
        //         "      Row(9, 0, 0, false),\n"
        //         "      Row(8, 0, 0, false),\n"
        //         "      Row(13, 3, 0, false),\n"
        //         "      Row(12, 3, 0, false),\n"
        //         "      Row(11, 3, 0, false),\n"
        //         "      Row(10, 3, 0, false),\n"
        //         "      Row(15, 6, 0, false),\n"
        //         "      Row(14, 6, 0, false),\n"

        //         // foo @ full (2, 4, 1)
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(1, 0, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(3, 3, 0, false),\n"
        //         "      Row(7, 6, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* grams1[] = { "foo", nullptr };
        //     Stream::Classification classification1 = Stream::Full;

        //     char* grams2[] = { "foobar", nullptr };
        //     Stream::Classification classification2 = Stream::MetaWord;

        //     expectedFalsePositiveEvaluationPlan
        //         << "And {\n"
        //         << "  Children: [\n"
        //         << "    Term {\n"
        //         << "      Children: FPMatchData(" << GetClassifiedHash(grams2, nullptr, classification2) << ", 1)\n"
        //         << "    },\n"
        //         << "    Term {\n"
        //         << "      Children: FPMatchData(" << GetClassifiedHash(grams1, nullptr, classification1) << ", 1)\n"
        //         << "    }\n"
        //         << "  ]\n"
        //         << "}";

        //     MockIndexConfiguration index(s_defaultShardCapacities);
        //     VerifyTermPlanConverterCase(input,
        //                                 expected,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 false);
        // }


        // TEST(TermPlanConverter,SimpleTreeWithSuffix)
        // {
        //     char const * input =
        //         "And {\n"
        //         "  Children: [\n"
        //         "    Unigram(\"foo\", full),\n"
        //         "    Unigram(\"foo\", full, \"Bar\")\n"
        //         "  ]\n"
        //         "}";

        //     char const * expected =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // foobar @ metaword (2, 4, 2)
        //         "      Row(9, 0, 0, false),\n"
        //         "      Row(8, 0, 0, false),\n"
        //         "      Row(13, 3, 0, false),\n"
        //         "      Row(12, 3, 0, false),\n"
        //         "      Row(11, 3, 0, false),\n"
        //         "      Row(10, 3, 0, false),\n"
        //         "      Row(15, 6, 0, false),\n"
        //         "      Row(14, 6, 0, false),\n"

        //         // foo @ full (2, 4, 1)
        //         "      Row(2, 0, 0, false),\n"
        //         "      Row(1, 0, 0, false),\n"
        //         "      Row(6, 3, 0, false),\n"
        //         "      Row(5, 3, 0, false),\n"
        //         "      Row(4, 3, 0, false),\n"
        //         "      Row(3, 3, 0, false),\n"
        //         "      Row(7, 6, 0, false),\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"
        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* grams1[] = { "foo", nullptr };
        //     Stream::Classification classification1 = Stream::Full;

        //     char* grams2[] = { "foo", nullptr };
        //     Stream::Classification classification2 = Stream::Full;

        //     expectedFalsePositiveEvaluationPlan
        //         << "And {\n"
        //         << "  Children: [\n"
        //         << "    Term {\n"
        //         << "      Children: FPMatchData(" << GetClassifiedHash(grams2, "Bar", classification2) << ", 1)\n"
        //         << "    },\n"
        //         << "    Term {\n"
        //         << "      Children: FPMatchData(" << GetClassifiedHash(grams1, nullptr, classification1) << ", 1)\n"
        //         << "    }\n"
        //         << "  ]\n"
        //         << "}";

        //     MockIndexConfiguration index(s_defaultShardCapacities);
        //     VerifyTermPlanConverterCase(input,
        //         expected,
        //         expectedFalsePositiveEvaluationPlan.str().c_str(),
        //         index,
        //         false);
        // }


        // TEST(TermPlanConverter,ComplexTreeUsingAllNodeTypes)
        // {
        //     char const * input =
        //         "And {\n"
        //         "  Children: [\n"
        //         "    Or {\n"
        //         "      Children: [\n"
        //         "        Unigram(\"foo\", clickboost),\n"
        //         "        Phrase {\n"
        //         "          Classification: nonbody,\n"
        //         "          Suffix: nullable(), \n"
        //         "          Grams: [\n"
        //         "            \"123\",\n"
        //         "            \"456\",\n"
        //         "            \"789\"\n"
        //         "          ]\n"
        //         "        }\n"
        //         "      ]\n"
        //         "    },\n"
        //         "    Not {\n"
        //         "      Child: And {\n"
        //         "        Children: [\n"
        //         "          Phrase {\n"
        //         "            Classification: full,\n"
        //         "            Suffix: nullable(), \n"
        //         "            Grams: [\n"
        //         "              \"foobar\",\n"
        //         "              \"bar\"\n"
        //         "            ]\n"
        //         "          },\n"
        //         "          Phrase {\n"
        //         "            Classification: full,\n"
        //         "            Suffix: nullable(\"Stream\"), \n"
        //         "            Grams: [\n"
        //         "              \"foobaz\",\n"
        //         "              \"baz\"\n"
        //         "            ]\n"
        //         "          },\n"
        //         "          Unigram(\"bpp\", nonbody)\n"
        //         "        ]\n"
        //         "      }\n"
        //         "    },\n"
        //         "    Unigram(\"zzz\", nonbody, \"Stream\")\n"
        //         "  ]\n"
        //         "}";

        //     char const * expected =
        //         "RowPlan {\n"
        //         "  Match: And {\n"
        //         "    Children: [\n"

        //         // zzzStream @ nonbody
        //         "      Row(80, 0, 0, false),\n"
        //         "      Row(79, 0, 0, false),\n"

        //         "      Not {\n"
        //         "        Child: And {\n"
        //         "          Children: [\n"

        //         "            Row(70, 0, 0, false),\n"
        //         "            Row(69, 0, 0, false),\n"
        //         "            Row(74, 3, 0, false),\n"
        //         "            Row(73, 3, 0, false),\n"
        //         "            Row(72, 3, 0, false),\n"
        //         "            Row(71, 3, 0, false),\n"
        //         "            Row(78, 6, 0, false),\n"
        //         "            Row(77, 6, 0, false),\n"
        //         "            Row(76, 6, 0, false),\n"
        //         "            Row(75, 6, 0, false),\n"

        //         "            Row(65, 0, 0, false),\n"
        //         "            Row(64, 0, 0, false),\n"
        //         "            Row(67, 3, 0, false),\n"
        //         "            Row(66, 3, 0, false),\n"
        //         "            Row(68, 6, 0, false),\n"

        //         "            Row(59, 0, 0, false),\n"
        //         "            Row(63, 3, 0, false),\n"
        //         "            Row(62, 3, 0, false),\n"
        //         "            Row(61, 3, 0, false),\n"
        //         "            Row(60, 3, 0, false),\n"

        //         // bpp @ nonbody (1, 3, 1)
        //         "            Row(54, 0, 0, false),\n"
        //         "            Row(57, 3, 0, false),\n"
        //         "            Row(56, 3, 0, false),\n"
        //         "            Row(55, 3, 0, false),\n"
        //         "            Row(58, 6, 0, false),\n"

        //         // bar @ click (2, 4, 1)
        //         "            Row(48, 0, 0, false),\n"
        //         "            Row(47, 0, 0, false),\n"
        //         "            Row(52, 3, 0, false),\n"
        //         "            Row(51, 3, 0, false),\n"
        //         "            Row(50, 3, 0, false),\n"
        //         "            Row(49, 3, 0, false),\n"
        //         "            Row(53, 6, 0, false),\n"

        //         // foobar,bar @ click (1, 1, 2)
        //         "            Row(43, 0, 0, false),\n"
        //         "            Row(44, 3, 0, false),\n"
        //         "            Row(46, 6, 0, false),\n"
        //         "            Row(45, 6, 0, false),\n"

        //         // foobar @ click (2, 0, 0)
        //         "            Row(42, 0, 0, false),\n"
        //         "            Row(41, 0, 0, false)\n"
        //         "          ]\n"
        //         "        }\n"
        //         "      },\n"
        //         "      Or {\n"
        //         "        Children: [\n"
        //         "          And {\n"
        //         "            Children: [\n"
        //         // 789 @ nonbody (1, 2, 1)
        //         "              Row(37, 0, 0, false),\n"
        //         "              Row(39, 3, 0, false),\n"
        //         "              Row(38, 3, 0, false),\n"
        //         "              Row(40, 6, 0, false),\n"

        //         // 456,789 @ nonbody (1, 1, 4)
        //         "              Row(31, 0, 0, false),\n"
        //         "              Row(32, 3, 0, false),\n"
        //         "              Row(36, 6, 0, false),\n"
        //         "              Row(35, 6, 0, false),\n"
        //         "              Row(34, 6, 0, false),\n"
        //         "              Row(33, 6, 0, false),\n"

        //         // 456 @ nonbody (2, 0, 0)
        //         "              Row(30, 0, 0, false),\n"
        //         "              Row(29, 0, 0, false),\n"

        //         // 123,456,789 @ nonbody (1, 3, 4)
        //         "              Row(21, 0, 0, false),\n"
        //         "              Row(24, 3, 0, false),\n"
        //         "              Row(23, 3, 0, false),\n"
        //         "              Row(22, 3, 0, false),\n"
        //         "              Row(28, 6, 0, false),\n"
        //         "              Row(27, 6, 0, false),\n"
        //         "              Row(26, 6, 0, false),\n"
        //         "              Row(25, 6, 0, false),\n"

        //         // 123,456 @ nonbody (1, 3, 1)
        //         "              Row(16, 0, 0, false),\n"
        //         "              Row(19, 3, 0, false),\n"
        //         "              Row(18, 3, 0, false),\n"
        //         "              Row(17, 3, 0, false),\n"
        //         "              Row(20, 6, 0, false),\n"

        //         // 123 @ nonbody (2, 4, 2)
        //         "              Row(9, 0, 0, false),\n"
        //         "              Row(8, 0, 0, false),\n"
        //         "              Row(13, 3, 0, false),\n"
        //         "              Row(12, 3, 0, false),\n"
        //         "              Row(11, 3, 0, false),\n"
        //         "              Row(10, 3, 0, false),\n"
        //         "              Row(15, 6, 0, false),\n"
        //         "              Row(14, 6, 0, false)\n"
        //         "            ]\n"
        //         "          },\n"
        //         "          And {\n"
        //         "            Children: [\n"

        //         // foo @ full (2, 4, 1)
        //         "              Row(2, 0, 0, false),\n"
        //         "              Row(1, 0, 0, false),\n"
        //         "              Row(6, 3, 0, false),\n"
        //         "              Row(5, 3, 0, false),\n"
        //         "              Row(4, 3, 0, false),\n"
        //         "              Row(3, 3, 0, false),\n"
        //         "              Row(7, 6, 0, false)\n"
        //         "            ]\n"
        //         "          }\n"
        //         "        ]\n"
        //         "      },\n"

        //         // Soft-deleted row.
        //         "      Row(0, 0, 0, false)\n"

        //         "    ]\n"
        //         "  }\n"
        //         "}";

        //     std::stringstream expectedFalsePositiveEvaluationPlan;

        //     char* grams1[] = { "bpp", nullptr };
        //     char* grams2[] = { "foobar", "bar", nullptr };
        //     char* grams3[] = { "foobar", nullptr };
        //     char* grams4[] = { "bar", nullptr };
        //     char* grams5[] = { "foo", nullptr };
        //     char* grams21[] = { "zzz", nullptr };
        //     char* grams22[] = { "foobaz", "baz", nullptr };
        //     char* grams23[] = { "foobaz", nullptr };
        //     char* grams24[] = { "baz", nullptr };
        //     Stream::Classification classification1 = Stream::Full;
        //     Stream::Classification classification2 = Stream::Click;

        //     char* wholePhrase[] = { "123", "456", "789", nullptr };
        //     char* subPhrase1[] = { "123", nullptr };
        //     char* subPhrase2[] = { "123", "456", nullptr };
        //     char* subPhrase3[] = { "456", nullptr };
        //     char* subPhrase4[] = { "456", "789", nullptr };
        //     char* subPhrase5[] = { "789", nullptr };
        //     Stream::Classification classification3 = Stream::NonBody;

        //     expectedFalsePositiveEvaluationPlan
        //         << "And {\n"
        //         << "  Children: [\n"
        //         << "    Term {\n"
        //         << "      Children: FPMatchData(" << GetClassifiedHash(grams21, "Stream", classification3) << ", 1)\n"
        //         << "    },\n"
        //         << "    Not {\n"
        //         << "      Child: And {\n"
        //         << "        Children: [\n"
        //         << "          Term {\n"
        //         << "            Children: FPMatchData(" << GetClassifiedHash(grams1, nullptr, classification3) << ", 1)\n"
        //         << "          },\n"
        //         << "          Phrase {\n"
        //         << "            Base: FPMatchData(" << GetClassifiedHash(grams22, "Stream", classification1) << ", 2),\n"
        //         << "            Subphrases: [\n"
        //         << "              FPMatchData(" << GetClassifiedHash(grams23, "Stream", classification1) << ", 1),\n"
        //         << "              FPMatchData(" << GetClassifiedHash(grams24, "Stream", classification1) << ", 1)\n"
        //         << "            ]\n"
        //         << "          },\n"
        //         << "          Phrase {\n"
        //         << "            Base: FPMatchData(" << GetClassifiedHash(grams2, nullptr, classification1) << ", 2),\n"
        //         << "            Subphrases: [\n"
        //         << "              FPMatchData(" << GetClassifiedHash(grams3, nullptr, classification1) << ", 1),\n"
        //         << "              FPMatchData(" << GetClassifiedHash(grams4, nullptr, classification1) << ", 1)\n"
        //         << "            ]\n"
        //         << "          }\n"
        //         << "        ]\n"
        //         << "      }\n"
        //         << "    },\n"
        //         << "    Or {\n"
        //         << "      Children: [\n"
        //         << "        Phrase {\n"
        //         << "          Base: FPMatchData(" << GetClassifiedHash(wholePhrase, nullptr, classification3) << ", 3),\n"
        //         << "          Subphrases: [\n"
        //         << "            FPMatchData(" << GetClassifiedHash(subPhrase1, nullptr, classification3) << ", 1),\n"
        //         << "            FPMatchData(" << GetClassifiedHash(subPhrase2, nullptr, classification3) << ", 2),\n"
        //         << "            FPMatchData(" << GetClassifiedHash(subPhrase3, nullptr, classification3) << ", 1),\n"
        //         << "            FPMatchData(" << GetClassifiedHash(subPhrase4, nullptr, classification3) << ", 2),\n"
        //         << "            FPMatchData(" << GetClassifiedHash(subPhrase5, nullptr, classification3) << ", 1)\n"
        //         << "          ]\n"
        //         << "        },\n"
        //         << "        Term {\n"
        //         << "          Children: FPMatchData(" << GetClassifiedHash(grams5, nullptr, classification2) << ", 1)\n"
        //         << "        }\n"
        //         << "      ]\n"
        //         << "    }\n"
        //         << "  ]\n"
        //         << "}";

        //     MockIndexConfiguration index(s_defaultShardCapacities);
        //     VerifyTermPlanConverterCase(input,
        //                                 expected,
        //                                 expectedFalsePositiveEvaluationPlan.str().c_str(),
        //                                 index,
        //                                 false);
        // }
    }
}
