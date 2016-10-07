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
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Term.h"
#include "ByteCodeInterpreter.h"
#include "ByteCodeVerifier.h"
#include "CompileNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    static const ShardId c_shardId = 0;
    static const Term::StreamId c_streamId = 0;
    static const size_t c_allocatorBufferSize = 1000000;


    ByteCodeVerifier::ByteCodeVerifier(ISimpleIndex const & index,
                                       Rank initialRank)
      : m_index(index),
        m_initialRank(initialRank),
        m_slices(index.GetIngestor().GetShard(c_shardId).GetSliceBuffers()),
        m_resultsCount(0),
        m_verboseMode(false),
        m_expectNoResults(false)
    {
        auto & shard = m_index.GetIngestor().GetShard(c_shardId);
        auto & sliceBuffers = shard.GetSliceBuffers();
        auto iterationsPerSlice = GetIterationsPerSlice();
        auto iterationCount = iterationsPerSlice * sliceBuffers.size();

        for (size_t i = 0; i < iterationCount; ++i)
        {
            m_iterationValues.push_back(i);
        }
    }


    void ByteCodeVerifier::VerboseMode(bool mode)
    {
        m_verboseMode = mode;
    }


    void ByteCodeVerifier::ExpectNoResults()
    {
        m_expectNoResults = true;
    }


    std::vector<size_t> const & ByteCodeVerifier::GetIterations() const
    {
        return m_iterationValues;
    }


    size_t ByteCodeVerifier::GetSliceNumber(size_t iteration) const
    {
        return iteration / GetIterationsPerSlice();
    }


    size_t ByteCodeVerifier::GetOffset(size_t iteration) const
    {
        return iteration % GetIterationsPerSlice();
    }


    size_t ByteCodeVerifier::GetIterationsPerSlice() const
    {
        auto & shard = m_index.GetIngestor().GetShard(c_shardId);
        return shard.GetSliceCapacity() >> 6 >> m_initialRank;
    }


    void ByteCodeVerifier::DeclareRow(char const * text)
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


    uint64_t ByteCodeVerifier::GetRowData(size_t row, size_t offset, size_t slice)
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
    void ByteCodeVerifier::ExpectResult(uint64_t accumulator,
                                        size_t offset,
                                        size_t slice)
    {
        if (accumulator != 0)
        {
            m_expectedResults.push_back({ accumulator, offset, slice });
            if (m_verboseMode)
            {
                std::cout
                    << "Expect: " << std::hex << accumulator << std::dec
                    << ", " << slice
                    << ", " << offset << std::endl;
            }
        }
        else
        {
            if (m_verboseMode)
            {
                std::cout
                    << "XXXXXX: " << std::hex << accumulator << std::dec
                    << ", " << slice
                    << ", " << offset << std::endl;
            }
        }
    }


    //
    // IResultsProcessor methods.
    //

    void ByteCodeVerifier::AddResult(uint64_t accumulator,
                                     size_t offset)
    {
        if (m_verboseMode)
        {
            std::cout
                << "AddResult("
                << std::hex << accumulator << std::dec
                << ", " << offset
                << ")" << std::endl;
        }

        m_observed.push_back({ accumulator, offset });
    }


    bool ByteCodeVerifier::FinishIteration(void const * sliceBuffer)
    {
        if (m_verboseMode)
        {
            std::cout
                << "FinishIteration("
                << std::hex << sliceBuffer << std::dec
                << ")" << std::endl;
        }

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


    bool ByteCodeVerifier::TerminatedEarly() const
    {
        std::cout
            << "TerminatedEarly()" << std::endl;
        return false;
    }


    void ByteCodeVerifier::Verify(char const * codeText)
    {
        // TODO: This check isn't too useful at the moment because
        // document 0 matches all queries. Therefore, even a poorly
        // constructed test will get at least one match.
        if (m_expectNoResults)
        {
            ASSERT_EQ(m_expectedResults.size(), 0ull)
                << "Expecting no results, but ExpectResult() was called"
                "at least once with a non-zero accumulator value.";
        }
        else
        {
            ASSERT_GT(m_expectedResults.size(), 0ull)
                << "Expecting at least one result, but each call to "
                "ExpectResult() passed an empty accumulator.";
        }

        ByteCodeGenerator code;
        GenerateCode(codeText, code);
        code.Seal();

        auto & shard = m_index.GetIngestor().GetShard(c_shardId);
        auto & sliceBuffers = shard.GetSliceBuffers();
        auto iterationsPerSlice = GetIterationsPerSlice();

        ByteCodeInterpreter interpreter(
            code,
            *this,
            sliceBuffers.size(),
            reinterpret_cast<char* const *>(sliceBuffers.data()),
            iterationsPerSlice,
            m_rowOffsets.data());

        interpreter.Run();

        // This check is necessary to detect the case where the final call
        // to FinishIteration is missing.
        EXPECT_EQ(m_resultsCount, m_expectedResults.size());
    }


    //
    // static methods
    //

    void ByteCodeVerifier::GenerateCode(char const * rowPlanText,
                                        ByteCodeGenerator& code)
    {
        std::stringstream rowPlan(rowPlanText);

        Allocator allocator(c_allocatorBufferSize);
        TextObjectParser parser(rowPlan, allocator, &CompileNode::GetType);

        CompileNode const & node = CompileNode::Parse(parser);

        node.Compile(code);
    }


    RowId ByteCodeVerifier::GetFirstRow(ITermTable const & termTable,
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


    ptrdiff_t ByteCodeVerifier::GetRowOffset(
        char const * text,
        Term::StreamId stream,
        IConfiguration const & config,
        ITermTable const & termTable,
        IShard const & shard)
    {
        Term term(text, stream, config);
        RowId row = GetFirstRow(termTable, term);
        return shard.GetRowOffset(row);
    }
}
