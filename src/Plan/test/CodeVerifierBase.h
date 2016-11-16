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

#include <stddef.h>                     // size_t parameter.
#include <vector>                       // std::vector embedded.

#include "BitFunnel/BitFunnelTypes.h"   // Rank parameter, DocId template parameter.
#include "ICodeVerifier.h"              // Base class.
#include "BitFunnel/Index/RowId.h"      // RowId parameter.


namespace BitFunnel
{
    class ByteCodeGenerator;
    class IShard;
    class ISimpleIndex;
    class ResultsBuffer;

    //*************************************************************************
    //
    // CodeVerifierBase
    //
    // This class verifies the correctness of a matching algorithm.
    //
    //*************************************************************************
    class CodeVerifierBase : public ICodeVerifier
    {
    public:
        CodeVerifierBase(ISimpleIndex const & index, Rank initialRank);


        //
        // ICodeVerifier methods.
        //

        virtual void VerboseMode(bool mode) override;

        virtual void ExpectNoResults() override;

        virtual std::vector<size_t> const & GetIterations() const override;

        virtual size_t GetSliceNumber(size_t iteration) const override;

        virtual size_t GetOffset(size_t iteration) const override;

        virtual size_t GetIterationsPerSlice() const override;

        virtual void DeclareRow(char const * text) override;

        virtual uint64_t GetRowData(size_t row,
                                    size_t offset,
                                    size_t slice) override;

        // The class is configured by adding a sequence of of records
        // describing the expected interactions betweeen the matcher and its
        // IResultsProcessor. Each call to Add() corresponds to expectation
        // that the matcher will invoke its interal AddResult() method. The
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
        virtual void ExpectResult(uint64_t accumulator,
                                  size_t offset,
                                  size_t slice) override;


    protected:
        void CheckResults(ResultsBuffer const & results);

    private:
        static RowId GetFirstRow(ITermTable const & termTable,
                                 Term term);

        static ptrdiff_t GetRowOffset(char const * text,
                                      Term::StreamId stream,
                                      IConfiguration const & config,
                                      ITermTable const & termTable,
                                      IShard const & shard);


        //
        // Constructor parameters.
        //

    protected:
        ISimpleIndex const & m_index;
    private:
        Rank m_initialRank;

        //
        // Other parameters.
        //

    protected:
        std::vector<void *> const & m_slices;

    private:
        std::vector<size_t> m_iterationValues;

    protected:
        std::vector<ptrdiff_t> m_rowOffsets;

    protected:
        size_t m_resultsCount;

        bool m_verboseMode;

        bool m_expectNoResults;

        std::set<DocId> m_observed;

    private:
        std::set<DocId> m_expected;
    };
}
