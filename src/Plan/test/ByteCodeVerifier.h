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

#pragma once

#include <stddef.h>                             // size_t parameter.
#include <vector>                               // std::vector return value.

#include "BitFunnel/BitFunnelTypes.h"           // Rank parameter.
#include "BitFunnel/Index/RowId.h"              // RowId parameter.
#include "BitFunnel/Plan/IResultsProcessor.h"   // Base class.


namespace BitFunnel
{
    class ByteCodeGenerator;
    class IShard;
    class ISimpleIndex;

    //*************************************************************************
    //
    // ByteCodeVerifier
    //
    // This class verifies the correctness of a matching algorithm that invokes
    // methods on an IResultsProcessor.
    //
    // Verifies that a sequence of calls to IResultsProcessor::AddResult() and
    // IResultsProcessor::FinishIteration() match an expect sequence of calls.
    //
    //*************************************************************************
    class ByteCodeVerifier : public IResultsProcessor
    {
    public:
        ByteCodeVerifier(ISimpleIndex const & index, Rank initialRank);


        void VerboseMode(bool mode);

        void ExpectNoResults();

        std::vector<size_t> const & GetIterations() const;

        size_t GetSliceNumber(size_t iteration) const;

        size_t GetOffset(size_t iteration) const;

        size_t GetIterationsPerSlice() const;

        void DeclareRow(char const * text);

        uint64_t GetRowData(size_t row, size_t offset, size_t slice);

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
                          size_t slice);

        void Verify(char const * codeText);

        //
        // IResultsProcessor methods.
        //

        void AddResult(uint64_t accumulator,
                       size_t offset) override;


        bool FinishIteration(void const * sliceBuffer) override;


        bool TerminatedEarly() const override;



    private:
        static void GenerateCode(char const * rowPlanText,
                                 ByteCodeGenerator& code);


        static RowId GetFirstRow(ITermTable const & termTable,
                                 Term term);

        static ptrdiff_t GetRowOffset(char const * text,
                                      Term::StreamId stream,
                                      IConfiguration const & config,
                                      ITermTable const & termTable,
                                      IShard const & shard);


        ISimpleIndex const & m_index;
        Rank m_initialRank;
        std::vector<void *> const & m_slices;

        std::vector<size_t> m_iterationValues;

        std::vector<ptrdiff_t> m_rowOffsets;

        size_t m_resultsCount;

        bool m_verboseMode;
        bool m_expectNoResults;

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
}
