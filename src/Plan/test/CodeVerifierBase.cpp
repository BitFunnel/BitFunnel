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

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <iomanip>
#include <iostream>

#include "gtest/gtest.h"

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Utilities/Factories.h"  // TODO: only for diagnosticStream. Remove.
#include "ByteCodeInterpreter.h"
#include "CodeVerifierBase.h"


namespace BitFunnel
{
    static const ShardId c_shardId = 0;
    static const Term::StreamId c_streamId = 0;

    // TODO: move this method somewhere.
    static uint64_t lzcnt(uint64_t value)
    {
#ifdef _MSC_VER
        return __lzcnt64(value);
#elif __LZCNT__
        return __lzcnt64(value);
#else
        // DESIGN NOTE: this is undefined if the input operand i 0. We currently
        // only call lzcnt in one place, where we've prevously gauranteed that
        // the input isn't 0. This is not safe to use in the general case.
        return static_cast<uint64_t>(__builtin_clzll(value));
#endif
    }


    CodeVerifierBase::CodeVerifierBase(ISimpleIndex const & index,
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


    void CodeVerifierBase::VerboseMode(bool mode)
    {
        m_verboseMode = mode;
    }


    void CodeVerifierBase::ExpectNoResults()
    {
        m_expectNoResults = true;
    }


    std::vector<size_t> const & CodeVerifierBase::GetIterations() const
    {
        return m_iterationValues;
    }


    size_t CodeVerifierBase::GetSliceNumber(size_t iteration) const
    {
        return iteration / GetIterationsPerSlice();
    }


    size_t CodeVerifierBase::GetOffset(size_t iteration) const
    {
        return iteration % GetIterationsPerSlice();
    }


    size_t CodeVerifierBase::GetIterationsPerSlice() const
    {
        auto & shard = m_index.GetIngestor().GetShard(c_shardId);
        return shard.GetSliceCapacity() >> 6 >> m_initialRank;
    }


    void CodeVerifierBase::DeclareRow(char const * text)
    {
        auto & shard = m_index.GetIngestor().GetShard(c_shardId);
        m_rowOffsets.push_back(
            GetRowOffset(
                text,
                c_streamId,
                m_index.GetConfiguration(),
                m_index.GetTermTable(c_shardId),
                shard));
    }


    uint64_t CodeVerifierBase::GetRowData(size_t row, size_t offset, size_t slice)
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
    void CodeVerifierBase::ExpectResult(uint64_t accumulator,
                                        size_t offset,
                                        size_t sliceNumber)
    {
        //// TODO: Remove temporary debugging output.
        //std::cout
        //    << "ExpectResult("
        //    << std::hex << accumulator << std::dec
        //    << ", "
        //    << offset
        //    << ", "
        //    << sliceNumber
        //    << ")"
        //    << std::endl;

        size_t bitPos = 63;
        void * sliceBuffer = m_slices[sliceNumber];

        while (accumulator != 0)
        {
            uint64_t count = lzcnt(accumulator);
            accumulator <<= count;
            bitPos -= count;

            DocIndex docIndex = offset * c_bitsPerQuadword + bitPos;
            DocumentHandle handle =
                Factories::CreateDocumentHandle(sliceBuffer, docIndex);

            if (handle.IsActive())
            {
                DocId id = handle.GetDocId();
                //// TODO: Remove temporary debugging output.
                //std::cout << "  " << id << std::endl;

                if (m_expected.find(id) != m_expected.end())
                {
                    // std::cout << "  Duplicate id " << id << std::endl;
                }

                m_expected.insert(id);
            }

            accumulator <<= 1;
            --bitPos;
        }
    }


    void CodeVerifierBase::CheckResults(ResultsBuffer const & results)
    {
        for (auto result : results)
        {
            DocumentHandle handle =
                Factories::CreateDocumentHandle(result.m_slice, result.m_index);

            if (handle.IsActive())
            {
                DocId doc = handle.GetDocId();

                // TODO: Remove temporary debugging output.
                //std::cout
                //    << "  ==> " << doc << std::endl;

                m_observed.insert(doc);
            }
        }

        ASSERT_EQ(m_expected.size(), m_observed.size())
            << "Inconsistent match counts.";

        for (DocId d : m_expected)
        {
            auto it = m_observed.find(d);
            ASSERT_TRUE(it != m_observed.end())
                << "Expected docId "
                << d
                << " not found.";
        }
    }


    //
    // static methods
    //

    RowId CodeVerifierBase::GetFirstRow(ITermTable const & termTable,
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


    ptrdiff_t CodeVerifierBase::GetRowOffset(
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
