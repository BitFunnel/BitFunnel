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

#include <iomanip>
#include <iostream>

#include "gtest/gtest.h"

#include "Allocator.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Mocks/Factories.h"
#include "BitFunnel/Plan/IResultsProcessor.h"
#include "BitFunnel/Term.h"
#include "ByteCodeInterpreter.h"
#include "CompileNode.h"
#include "Primes.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // ExpectedResults maintains information about a sequence of calls to
    // IResultsProcessor::AddResult() and IResultsProcessor::FinishIteration().
    // This information is used to verify the correctness of a matching
    // algorithm that invokes methods on an IResultsProcessor.
    //
    //*************************************************************************
    class ExpectedResults
    {
    public:
        // The class is configured by adding a sequence of of records
        // describing the expected interactions betweeen the matcher and its
        // IResultsProcessor. Each call to Add() corresponds to expectation
        // that the matcher will invoke IResultsProcessor::AddResult(). The
        // accumulator and offset parameters to Add() are the expected
        // parameters to AddResult().
        //
        // The usage pattern for IResultsProcessor is a sequence of calls
        // to AddResult() interspersed with calls to FinishIteration().
        // The call to FinishIteration() provides the void* slice buffer
        // pointer that is applicable to the sequence of calls to AddResult()
        // since the previous call to FinishIteration() (or the start of
        // the matching algorithm if there was no previous call to
        // FinishIteration().
        //
        // The third parameter of the Add() method indicates the slice
        // that expected to be passed on the next call to FinishIteration.
        // The slice parameter is a size_t index into an array of slice
        // buffer pointers associated with the index.
        // 
        void Add(uint64_t accumulator,
                 size_t offset,
                 size_t slice)
        {
            if (accumulator != 0)
            {
                m_expected.push_back({ accumulator, offset, slice });
                std::cout
                    << "Expect: " << std::hex << accumulator << std::dec
                    << ", " << offset
                    << ", " << slice << std::endl;
            }
            else
            {
                std::cout
                    << "XXXXXX: " << std::hex << accumulator << std::dec
                    << ", " << offset
                    << ", " << slice << std::endl;
            }
        }


        void Check(size_t index,
                   uint64_t accumulator,
                   size_t offset,
                   void const * slice,
                   std::vector<void *> const & slices) const
        {
            // ASSERT, rather than EXPECT to avoid out of bounds array index.
            ASSERT_LT(index, m_expected.size());
            auto const & expected = m_expected[index];
            EXPECT_EQ(accumulator, expected.m_accumulator);
            EXPECT_EQ(offset, expected.m_offset);
            EXPECT_EQ(slice, slices[expected.m_slice]);
        }


        size_t GetResultCount() const
        {
            return m_expected.size();
        }

    private:
        struct Expected
        {
            uint64_t m_accumulator;
            size_t m_offset;
            size_t m_slice;
        };

        std::vector<Expected> m_expected;
    };


    //*************************************************************************
    //
    // VerifyingResultsProcessor
    //
    //*************************************************************************
    class VerifyingResultsProcessor : public IResultsProcessor
    {
    public:
        VerifyingResultsProcessor(/*ExpectedResults const & expected,*/
                                  std::vector<void *> const & slices)
          : m_iterationCount(0),
            m_resultsCount(0),
//            m_expected(expected),
            m_slices(slices)
        {
        }


        void ExpectResult(uint64_t accumulator,
                          size_t offset,
                          size_t slice)
        {
            if (accumulator != 0)
            {
                m_expectedResults.push_back({ accumulator, offset, slice });
                std::cout
                    << "Expect: " << std::hex << accumulator << std::dec
                    << ", " << offset
                    << ", " << slice << std::endl;
            }
            else
            {
                std::cout
                    << "XXXXXX: " << std::hex << accumulator << std::dec
                    << ", " << offset
                    << ", " << slice << std::endl;
            }
        }


        //
        // IResultsProcessor methods.
        //

        void AddResult(uint64_t accumulator,
                       size_t offset) override
        {
            std::cout
                << "AddResult("
                << std::hex << accumulator << std::dec
                << ", " << offset
                << ")" << std::endl;

            m_observed.push_back({ accumulator, offset });

            //// TODO: Dedupe
            //DocId id = 0;
            //while (accumulator != 0)
            //{
            //    if ((accumulator & 1) == 1)
            //    {
            //        m_results.push_back(id + (offset << 6));
            //    }
            //    accumulator >>= 1;
            //    ++id;
            //}
        }


        bool FinishIteration(void const * sliceBuffer) override
        {
            std::cout
                << "FinishIteration(" << m_iterationCount++ 
                << ", " << std::hex << sliceBuffer << std::dec
                << ")" << std::endl;

            for (size_t i = 0; i < m_observed.size(); ++i)
            {
                // Would like to ASSERT, rather than EXPECT to avoid out of
                // bounds array index below. Unfortunately ASSERT cannot be
                // used inside of a function that returns bool. Hence, we
                // EXPECT_LT and then break on failure.
                EXPECT_LT(m_resultsCount, m_expectedResults.size());
                if (m_resultsCount >= m_expectedResults.size())
                {
                    break;
                }

                auto const & expected = m_expectedResults[m_resultsCount];
                auto const & observed = m_observed[i];

                EXPECT_EQ(observed.m_accumulator, expected.m_accumulator);
                EXPECT_EQ(observed.m_offset, expected.m_offset);
                EXPECT_EQ(sliceBuffer, m_slices[expected.m_slice]);

                m_resultsCount++;
                //m_expected.Check(m_resultsCount++,
                //                 m_observed[i].m_accumulator,
                //                 m_observed[i].m_offset,
                //                 sliceBuffer,
                //                 m_slices);
            }


            //for (size_t i = 0; i < m_observed.size(); ++i)
            //{
            //    m_expected.Check(m_resultsCount++,
            //                     m_observed[i].m_accumulator,
            //                     m_observed[i].m_offset,
            //                     sliceBuffer,
            //                     m_slices);
            //}

            m_observed.clear();

            // TODO: Should this return true or false?
            return false;
        }


        bool TerminatedEarly() const override
        {
            std::cout
                << "TerminatedEarly()" << std::endl;
            return false;
        }


        //void PrintResults(std::ostream& output)
        //{
        //    output << "Matches: ";
        //    for (auto id : m_observed)
        //    {
        //        output << id << " ";
        //    }
        //    output << std::endl;
        //}


        void Check()
        {
            EXPECT_EQ(m_resultsCount, m_expectedResults.size());
//            EXPECT_EQ(m_resultsCount, m_expected.GetResultCount());
        }

    private:
        size_t m_iterationCount;
        size_t m_resultsCount;
//        ExpectedResults const & m_expected;
        std::vector<void *> const & m_slices;

        struct Expected
        {
            uint64_t m_accumulator;
            size_t m_offset;
            size_t m_slice;
        };

        std::vector<Expected> m_expectedResults;

        struct Observed
        {
            uint64_t m_accumulator;
            size_t m_offset;
        };

        std::vector<Observed> m_observed;

//        std::vector<DocId> m_results;
    };


    size_t c_allocatorBufferSize = 1000000;

    void GenerateCode(char const * rowPlanText,
                      ByteCodeGenerator& code)
    {
        std::stringstream rowPlan(rowPlanText);

        Allocator allocator(c_allocatorBufferSize);
        TextObjectParser parser(rowPlan, allocator, &CompileNode::GetType);

        CompileNode const & node = CompileNode::Parse(parser);

        node.Compile(code);
    }


    RowId GetFirstRow(ITermTable const & termTable,
                      Term term)
    {
        RowIdSequence rows(term, termTable);

        auto it = rows.begin();
        // TODO: Implement operator << for RowIdSequence::const_iterator.
        //CHECK_NE(it, rows.end())
        //    << "Expected at least one row.";

        RowId row =  *it;

        ++it;
        // TODO: Implement operator << for RowIdSequence::const_iterator.
        //CHECK_EQ(it, rows.end())
        //    << "Expected no more than one row.";

        return row;
    }


    //ptrdiff_t GetRowOffset(char const * text,
    //                       Term::StreamId stream,
    //                       IConfiguration const & config,
    //                       ITermTable const & termTable,
    //                       IShard const & shard)
    //{
    //    Term term(text, stream, config);
    //    RowId row = GetFirstRow(termTable, term);
    //    return shard.GetRowOffset(row);
    //}

    class MockIndex
    {
    public:
        static const DocId maxDocId = 831;
        static const Term::StreamId streamId = 0;
        static const size_t maxGramSize = 1;
        static const Rank c_maxRank = 0;
        static const ShardId c_shardId = 0;


        MockIndex()
        {
            m_fileSystem = Factories::CreateRAMFileSystem();

            m_index = Factories::CreatePrimeFactorsIndex(
                *m_fileSystem, maxDocId, streamId);

            const size_t c_rowCount = 6;

            auto & shard = m_index->GetIngestor().GetShard(c_shardId);

            for (size_t i = 0; i < c_rowCount; ++i)
            {
                char const * term;
                if (i == 0)
                {
                    term = "0";
                }
                else if (i == 1)
                {
                    term = "1";
                }
                else
                {
                    term = Primes::c_primesBelow10000Text[i - 2].c_str();
                }
                m_rowOffsets.push_back(
                    GetRowOffset(
                        term,
                        streamId,
                        m_index->GetConfiguration(),
                        m_index->GetTermTable(),
                        shard));
            }
        }


        void RunTest(char const * codeText,
                     VerifyingResultsProcessor & resultsProcessor)
        {
            ByteCodeGenerator code;
            GenerateCode(codeText, code);
            code.Seal();

            auto & shard = m_index->GetIngestor().GetShard(c_shardId);
            auto & sliceBuffers = shard.GetSliceBuffers();
            auto iterationsPerSlice = shard.GetSliceCapacity() / (64ull << c_maxRank);

            ByteCodeInterpreter interpreter(
                code,
                resultsProcessor,
                sliceBuffers.size(),
                reinterpret_cast<char* const *>(sliceBuffers.data()),
                iterationsPerSlice,
                m_rowOffsets.data());

            interpreter.Run();

            resultsProcessor.Check();
        }


        std::vector<void *> const & GetSliceBuffers() const
        {
            auto & shard = m_index->GetIngestor().GetShard(c_shardId);
            return shard.GetSliceBuffers();
        }

    private:
        static ptrdiff_t GetRowOffset(char const * text,
                                      Term::StreamId stream,
                                      IConfiguration const & config,
                                      ITermTable const & termTable,
                                      IShard const & shard)
        {
            Term term(text, stream, config);
            RowId row = GetFirstRow(termTable, term);
            return shard.GetRowOffset(row);
        }

        std::unique_ptr<IFileSystem> m_fileSystem;
        std::unique_ptr<ISimpleIndex> m_index;
        std::vector<ptrdiff_t> m_rowOffsets;
    };


//    void RunTest(ByteCodeGenerator const & code,
//                 VerifyingResultsProcessor & resultsProcessor)
//    {
//        // TODO: Verify reason for crash with maxDocId == 832.
//        // Think it is hard-coded iteration count.
//        const DocId maxDocId = 831;
//        const Term::StreamId streamId = 0;
//
//        const size_t maxGramSize = 1;
//
//        auto fileSystem = Factories::CreateRAMFileSystem();
//
//        auto index = Factories::CreatePrimeFactorsIndex(
//            *fileSystem, maxDocId, streamId);
//
//        const ShardId shardId = 0;
//        auto & shard = index->GetIngestor().GetShard(shardId);
//
//        std::vector<ptrdiff_t> rowOffsets;
//
//        rowOffsets.push_back(GetRowOffset(
//            "0",
//            streamId,
//            index->GetConfiguration(),
//            index->GetTermTable(),
//            shard));
//        rowOffsets.push_back(GetRowOffset(
//            "1",
//            streamId,
//            index->GetConfiguration(),
//            index->GetTermTable(),
//            shard));
//        rowOffsets.push_back(GetRowOffset(
//            "2",
//            streamId,
//            index->GetConfiguration(),
//            index->GetTermTable(),
//            shard));
//
//        Rank c_maxRank = 0;
//
//        auto & sliceBuffers = shard.GetSliceBuffers();
//        auto iterationsPerSlice = shard.GetSliceCapacity() / (64ull << c_maxRank);
//
////        VerifyingResultsProcessor resultsProcessor(sliceBuffers);
//
//        ByteCodeInterpreter interpreter(
//            code,
//            resultsProcessor,
//            sliceBuffers.size(),
//            reinterpret_cast<char* const *>(sliceBuffers.data()),
//            iterationsPerSlice,
//            rowOffsets.data());
//
//        interpreter.Run();
//
//        resultsProcessor.Check();
//
////        resultsProcessor.PrintResults(std::cout);
//    }


    TEST(ByteCodeInterpreter, AndRowJzDelta0)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"      // Row(0) is 0, 1, 2, ...
            "  Child: AndRowJz {"
            "    Row: Row(2, 0, 0, false),"    // Row(2) is AAAAAAA....
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";


        MockIndex index;

        // Expect 9 results.
        //           X   X X   X X X   X     X X
        //  Row 0: 0 1 2 3 4   5 6 7 8 9   A B C
        //  Row 2: 5 5 5 5 5   5 5 5 5 5   5 5 5
        // Result:   1   1 4   5 4 5   1     1 4
        // Offset: 0 1 2 3 4   0 1 2 3 4   0 1 2
        //  Slice: 0 0 0 0 0   1 1 1 1 1   2 2 2
        const uint64_t row2 = 0x5555555555555555ull;

        VerifyingResultsProcessor expected(index.GetSliceBuffers());
        for (size_t iteration = 0; iteration < 13; ++iteration)
        {
            const size_t slice = iteration / 5;
            const size_t offset = iteration % 5;
            const uint64_t row0 = (slice * 5) + offset;
            expected.ExpectResult(row2 & row0, offset, slice);
        }

        index.RunTest(text, expected);
//        RunTest(code, expected);
    }


    TEST(ByteCodeInterpreter, AndRowJzDelta0Inverted)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"      // Row(0) is 0, 1, 2, ...
            "  Child: AndRowJz {"
            "    Row: Row(2, 0, 0, true),"     // Row(2) is AAAAAAA....
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        MockIndex index;

        const uint64_t row2 = 0x5555555555555555ull;

        VerifyingResultsProcessor expected(index.GetSliceBuffers());
        for (size_t iteration = 0; iteration < 13; ++iteration)
        {
            const size_t slice = iteration / 5;
            const size_t offset = iteration % 5;
            const uint64_t row0 = (slice * 5) + offset;
            expected.ExpectResult(~row2 & row0, offset, slice);
        }

        index.RunTest(text, expected);
    }


    TEST(ByteCodeInterpreter, AndRowJzDelta1)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(2, 0, 0, false),"      // Row(2) is AAAAAAA....
            "  Child: AndRowJz {"
            "    Row: Row(0, 0, 1, false),"    // Row(0) is 0, 1, 2, ...
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        MockIndex index;

        const uint64_t row2 = 0x5555555555555555ull;

        VerifyingResultsProcessor expected(index.GetSliceBuffers());
        for (size_t iteration = 0; iteration < 13; ++iteration)
        {
            const size_t slice = iteration / 5;
            const size_t offset = iteration % 5;
            const uint64_t row0 = (slice * 5) + offset / 2;
            expected.ExpectResult(row2 & row0, offset, slice);
        }

        index.RunTest(text, expected);
    }


    TEST(ByteCodeInterpreter, AndRowJzDelta1Inverted)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(2, 0, 0, false),"      // Row(2) is AAAAAAA....
            "  Child: AndRowJz {"
            "    Row: Row(0, 0, 1, true),"     // Row(0) is 0, 1, 2, ...
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        MockIndex index;

        const uint64_t row2 = 0x5555555555555555ull;

        VerifyingResultsProcessor expected(index.GetSliceBuffers());
        for (size_t iteration = 0; iteration < 13; ++iteration)
        {
            const size_t slice = iteration / 5;
            const size_t offset = iteration % 5;
            const uint64_t row0 = (slice * 5) + offset / 2;
            expected.ExpectResult(row2 & ~row0, offset, slice);
        }

        index.RunTest(text, expected);
    }

    // TODO: Expected loop needs to refer to actual row data values.
    //       Need fixture to get access to index that was built before all tests.
    // TODO: Code generation can be moved into RunTest

    //TEST(ByteCodeInterpreter, AndRowJzMatches)
    //{
    //    char const * text =
    //        "LoadRowJz {"
    //        "  Row: Row(2, 0, 0, false),"
    //        "  Child: AndRowJz {"
    //        "    Row: Row(3, 0, 0, false),"
    //        "    Child: AndRowJz {"
    //        "      Row: Row(5, 0, 0, false),"
    //        "      Child: Report {"
    //        "        Child: "
    //        "      }"
    //        "    }"
    //        "  }"
    //        "}";

    //    ByteCodeGenerator code;
    //    GenerateCode(text, code);
    //    code.Seal();

    //    const uint64_t row2 = 0x5555555555555555ull;

    //    ExpectedResults expected;
    //    for (size_t index = 0; index < 13; ++index)
    //    {
    //        const size_t slice = index / 5;
    //        const size_t offset = index % 5;
    //        const uint64_t row0 = (slice * 5) + offset / 2;
    //        expected.Add(row2 & ~row0, offset, slice);
    //    }

    //    RunTest(code, expected);
    //}
}
