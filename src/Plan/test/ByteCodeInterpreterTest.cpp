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
    std::unique_ptr<IFileSystem> g_fileSystem;
    std::unique_ptr<ISimpleIndex> g_index;
    const DocId c_maxDocId = 832;
    const Term::StreamId c_streamId = 0;
    static const ShardId c_shardId = 0;


    ISimpleIndex const & GetIndex()
    {
        if (g_fileSystem.get() == nullptr)
        {
            g_fileSystem = Factories::CreateRAMFileSystem();
        }

        if (g_index.get() == nullptr)
        {
            g_index = Factories::CreatePrimeFactorsIndex(*g_fileSystem,
                                                         c_maxDocId,
                                                         c_streamId);
        }

        return *g_index;
    }


    //*************************************************************************
    //
    // VerifyingResultsProcessor
    //
    // This class verifies the correctness of a matching algorithm that invokes
    // methods on an IResultsProcessor.
    //
    // Verifies that a sequence of calls to IResultsProcessor::AddResult() and
    // IResultsProcessor::FinishIteration() match an expect sequence of calls.
    //
    //*************************************************************************
    class Verifier : public IResultsProcessor
    {
    public:
        //static const ShardId c_shardId = 0;
//        static const Term::StreamId c_streamId = 0;
        static const size_t c_allocatorBufferSize = 1000000;


        Verifier(ISimpleIndex const & index, Rank initialRank)
          : m_index(index),
            m_initialRank(initialRank),
            m_slices(index.GetIngestor().GetShard(c_shardId).GetSliceBuffers()),
            m_resultsCount(0)
        {
        }


        size_t GetIterations() const
        {
            auto & shard = m_index.GetIngestor().GetShard(c_shardId);
            auto & sliceBuffers = shard.GetSliceBuffers();
            auto iterationsPerSlice = GetIterationsPerSlice();
            return iterationsPerSlice * sliceBuffers.size();
        }


        size_t GetSliceNumber(size_t iteration) const
        {
            return iteration / GetIterationsPerSlice();
        }


        size_t GetOffset(size_t iteration) const
        {
            return iteration % GetIterationsPerSlice();
        }


        // Returns number of iterations per slice at rank 0.
        size_t GetIterationsPerSlice() const
        {
            auto & shard = m_index.GetIngestor().GetShard(c_shardId);
            return shard.GetSliceCapacity() >> 6 >> m_initialRank;
        }


        void DeclareRow(char const * text)
        {
            auto & shard = m_index.GetIngestor().GetShard(c_shardId);
            m_rowOffsets.push_back(
                GetRowOffset(
                    text,
                    c_streamId,
                    m_index.GetConfiguration(),
                    m_index.GetTermTable(),
                    shard));
        }


        uint64_t GetRowData(size_t row, size_t slice, size_t offset)
        {
            auto & shard = m_index.GetIngestor().GetShard(c_shardId);
            auto const & slices = shard.GetSliceBuffers();
            char const * sliceBuffer = reinterpret_cast<char const *>(slices[slice]);
            uint64_t const * rowPtr =
                reinterpret_cast<uint64_t const *>(sliceBuffer + m_rowOffsets[row]);
            return rowPtr[offset];
        }


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
        }


        bool FinishIteration(void const * sliceBuffer) override
        {
            std::cout
                << "FinishIteration("
                << std::hex << sliceBuffer << std::dec
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
            }


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


        void Check()
        {
            EXPECT_EQ(m_resultsCount, m_expectedResults.size());
        }


        void Verify(char const * codeText)
        {
            ByteCodeGenerator code;
            GenerateCode(codeText, code);
            code.Seal();

            auto & shard = m_index.GetIngestor().GetShard(c_shardId);
            auto & sliceBuffers = shard.GetSliceBuffers();
            auto iterationsPerSlice = GetIterationsPerSlice();

            //std::vector<ptrdiff_t> rowOffsets;
            //for (RowId row : m_rows)
            //{
            //    rowOffsets.push_back(
            //        GetRowOffset(
            //            std::to_string(row.GetIndex()).c_str(),
            //            c_streamId,
            //            m_index.GetConfiguration(),
            //            m_index.GetTermTable(),
            //            shard));
            //}

            ByteCodeInterpreter interpreter(
                code,
                *this,
                sliceBuffers.size(),
                reinterpret_cast<char* const *>(sliceBuffers.data()),
                iterationsPerSlice,
                m_rowOffsets.data());

            interpreter.Run();

            Check();
        }

    private:
        static void GenerateCode(char const * rowPlanText,
                                 ByteCodeGenerator& code)
        {
            std::stringstream rowPlan(rowPlanText);

            Allocator allocator(c_allocatorBufferSize);
            TextObjectParser parser(rowPlan, allocator, &CompileNode::GetType);

            CompileNode const & node = CompileNode::Parse(parser);

            node.Compile(code);
        }


        static RowId GetFirstRow(ITermTable const & termTable,
                                 Term term)
        {
            RowIdSequence rows(term, termTable);

            auto it = rows.begin();
            // TODO: Implement operator << for RowIdSequence::const_iterator.
            //CHECK_NE(it, rows.end())
            //    << "Expected at least one row.";

            RowId row = *it;

            ++it;
            // TODO: Implement operator << for RowIdSequence::const_iterator.
            //CHECK_EQ(it, rows.end())
            //    << "Expected no more than one row.";

            return row;
        }


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


        ISimpleIndex const & m_index;
        Rank m_initialRank;
        std::vector<void *> const & m_slices;

//        std::vector<RowId> m_rows;
        std::vector<ptrdiff_t> m_rowOffsets;

        size_t m_resultsCount;

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
    };



    //*************************************************************************
    //
    // MockIndex
    //
    //*************************************************************************
    class MockIndex
    {
    public:
        static const DocId maxDocId = 831;
        static const Term::StreamId streamId = 0;
        static const size_t maxGramSize = 1;
        static const Rank c_maxRank = 0;
        static const ShardId c_shardId = 0;
        const size_t c_rowCount = 20;

        static const size_t c_allocatorBufferSize = 1000000;


        MockIndex()
        {
            m_fileSystem = Factories::CreateRAMFileSystem();

            m_index = Factories::CreatePrimeFactorsIndex(
                *m_fileSystem, maxDocId, streamId);

            auto & shard = m_index->GetIngestor().GetShard(c_shardId);

            for (size_t i = 0; i < c_rowCount; ++i)
            {
                //char const * term;
                //if (i == 0)
                //{
                //    term = "0";
                //}
                //else if (i == 1)
                //{
                //    term = "1";
                //}
                //else
                //{
                //    term = Primes::c_primesBelow10000Text[i - 2].c_str();
                //}
                m_rowOffsets.push_back(
                    GetRowOffset(
                        Primes::c_primesBelow10000Text[i].c_str(),
                        streamId,
                        m_index->GetConfiguration(),
                        m_index->GetTermTable(),
                        shard));
            }
        }


        void RunTest(char const * codeText,
                     Verifier & resultsProcessor,
                     Rank initialRank)
        {
            ByteCodeGenerator code;
            GenerateCode(codeText, code);
            code.Seal();

            auto & shard = m_index->GetIngestor().GetShard(c_shardId);
            auto & sliceBuffers = shard.GetSliceBuffers();
            auto iterationsPerSlice = GetIterationsPerSlice(initialRank);

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


        uint64_t GetRow(size_t row, size_t slice, size_t offset)
        {
            auto const & slices = GetSliceBuffers();
            char const * sliceBuffer = reinterpret_cast<char const *>(slices[slice]);
            uint64_t const * rowPtr =
                reinterpret_cast<uint64_t const *>(sliceBuffer + m_rowOffsets[row]);
            return rowPtr[offset];
        }


        size_t GetSliceNumber(size_t iteration, Rank initialRank) const
        {
            return iteration / GetIterationsPerSlice(initialRank);
        }


        size_t GetOffset(size_t iteration, Rank initialRank) const
        {
            return iteration % GetIterationsPerSlice(initialRank);
        }


        size_t GetIterations(Rank initialRank) const
        {
            auto & shard = m_index->GetIngestor().GetShard(c_shardId);
            auto & sliceBuffers = shard.GetSliceBuffers();
            auto iterationsPerSlice = GetIterationsPerSlice(initialRank);
            return iterationsPerSlice * sliceBuffers.size();
        }


        ISimpleIndex const & GetIndex() const
        {
            return *m_index;
        }


    private:
        // Returns number of iterations per slice at rank 0.
        size_t GetIterationsPerSlice(Rank initialRank) const
        {
            auto & shard = m_index->GetIngestor().GetShard(c_shardId);
            return shard.GetSliceCapacity() >> 6 >> initialRank;
        }


        static RowId GetFirstRow(ITermTable const & termTable,
                                 Term term)
        {
            RowIdSequence rows(term, termTable);

            auto it = rows.begin();
            // TODO: Implement operator << for RowIdSequence::const_iterator.
            //CHECK_NE(it, rows.end())
            //    << "Expected at least one row.";

            RowId row = *it;

            ++it;
            // TODO: Implement operator << for RowIdSequence::const_iterator.
            //CHECK_EQ(it, rows.end())
            //    << "Expected no more than one row.";

            return row;
        }


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


        static void GenerateCode(char const * rowPlanText,
                                 ByteCodeGenerator& code)
        {
            std::stringstream rowPlan(rowPlanText);

            Allocator allocator(c_allocatorBufferSize);
            TextObjectParser parser(rowPlan, allocator, &CompileNode::GetType);

            CompileNode const & node = CompileNode::Parse(parser);

            node.Compile(code);
        }


        std::unique_ptr<IFileSystem> m_fileSystem;
        std::unique_ptr<ISimpleIndex> m_index;
        std::vector<ptrdiff_t> m_rowOffsets;
    };


    //*************************************************************************
    //
    // Test cases
    //
    //*************************************************************************

    TEST(ByteCodeInterpreter, AndRowJzDelta0)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(1, 0, 0, false),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";


        const Rank initialRank = 0;
        Verifier verifier(GetIndex(), initialRank);

        verifier.DeclareRow("2");
        verifier.DeclareRow("3");

        for (size_t iteration = 0;
             iteration < verifier.GetIterations();
             ++iteration)
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, slice, offset);
            const uint64_t row1 = verifier.GetRowData(1, slice, offset);
            verifier.ExpectResult(row0 & row1, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(ByteCodeInterpreter, AndRowJzDelta0Inverted)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(1, 0, 0, true),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        Verifier verifier(GetIndex(), initialRank);

        verifier.DeclareRow("2");
        verifier.DeclareRow("3");

        for (size_t iteration = 0; iteration < verifier.GetIterations(); ++iteration)
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, slice, offset);
            const uint64_t row1 = verifier.GetRowData(1, slice, offset);
            verifier.ExpectResult(row0 & ~row1, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(ByteCodeInterpreter, AndRowJzDelta1)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(1, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(0, 0, 1, false),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        Verifier verifier(GetIndex(), initialRank);

        // IMPORTANT: row0 must be a row whose value differs
        // across adjacent quadwords in order to correctly
        // test the rank delta. If row0 has the same value in
        // adjacent quadwords,
        //     GetRowData(0, slice, offset)
        // will return the same value as
        //     GetRowData(0, slice, offset /2);
        verifier.DeclareRow("3");
        verifier.DeclareRow("2");

        for (size_t iteration = 0; iteration < verifier.GetIterations(); ++iteration)
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row1 = verifier.GetRowData(1, slice, offset);
            const uint64_t row0 = verifier.GetRowData(0, slice, offset / 2);
            verifier.ExpectResult(row1 & row0, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(ByteCodeInterpreter, AndRowJzDelta1Inverted)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(1, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(0, 0, 1, true),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        Verifier verifier(GetIndex(), initialRank);

        // IMPORTANT: row0 must be a row whose value differs
        // across adjacent quadwords in order to correctly
        // test the rank delta. If row0 has the same value in
        // adjacent quadwords,
        //     GetRowData(0, slice, offset)
        // will return the same value as
        //     GetRowData(0, slice, offset /2);
        verifier.DeclareRow("3");
        verifier.DeclareRow("2");

        for (size_t iteration = 0; iteration < verifier.GetIterations(); ++iteration)
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row1 = verifier.GetRowData(1, slice, offset);
            const uint64_t row0 = verifier.GetRowData(0, slice, offset / 2);
            verifier.ExpectResult(row1 & ~row0, offset, slice);
        }

        verifier.Verify(text);
    }

    //// TODO: Expected loop needs to refer to actual row data values.
    ////       Need fixture to get access to index that was built before all tests.
    //// TODO: Code generation can be moved into RunTest

    TEST(ByteCodeInterpreter, AndRowJzMatches)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(1, 0, 0, false),"
            "    Child: AndRowJz {"
            "      Row: Row(2, 0, 0, false),"
            "      Child: Report {"
            "        Child: "
            "      }"
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        Verifier verifier(GetIndex(), initialRank);

        verifier.DeclareRow("2");
        verifier.DeclareRow("3");
        verifier.DeclareRow("5");

        for (size_t iteration = 0; iteration < verifier.GetIterations(); ++iteration)
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, slice, offset);
            const uint64_t row1 = verifier.GetRowData(1, slice, offset);
            const uint64_t row2 = verifier.GetRowData(2, slice, offset);
            verifier.ExpectResult(row0 & row1 & row2, offset, slice);
        }

        verifier.Verify(text);
    }


    //TEST(ByteCodeInterpreter, RankDownDelta1)
    //{
    //    char const * text =
    //        "RankDown {"
    //        "  Delta: 1,"
    //        "  Child: LoadRowJz {"
    //        "    Row: Row(0, 5, 0, false),"
    //        "    Child: Report {"
    //        "      Child: "
    //        "    }"
    //        "  }"
    //        "}";

    //    MockIndex index;
    //    const Rank c_initialRank = 1;

    //    VerifyingResultsProcessor expected(index.GetSliceBuffers());
    //    for (size_t iteration = 0; iteration < index.GetIterations(c_initialRank); ++iteration)
    //    {
    //        const size_t slice = index.GetSliceNumber(iteration);
    //        const size_t offset = index.GetOffset(iteration);

    //        for (size_t i = 0; i < 2; ++i)
    //        {
    //            const uint64_t row0 = index.GetRow(0, slice, 2 * offset + i);
    //            expected.ExpectResult(row0, offset, slice);
    //        }
    //    }

    //    index.RunTest(text, expected, c_initialRank);
    //}
}
