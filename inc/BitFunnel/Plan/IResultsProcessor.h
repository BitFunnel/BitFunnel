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

#include <stdint.h>                 // uint64_t parameter.

#include "BitFunnel/IInterface.h"   // Base class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // IResultsProcessor is an abstract base class or interface that accepts
    // results from the matching engine and dedupes and then scores unique
    // results at the end of each iteration.
    //
    //*************************************************************************
    class IResultsProcessor : public IInterface
    {
    public:
        // Adds a match result to the dedupe buffer for the current matcher
        // iteration. The matcher iterates once for each quad word in the
        // highest rank row. The accumulator parameter will contain the
        // result of evaluating the boolean match tree on quadwords
        // corresponding to specified offset in each row. Note that offset
        // specifies the offset into a rank-0 row.
        virtual void AddResult(uint64_t accumulator,
                               size_t offset) = 0;

        // Instructs the ResultsProcessor to dedupe the results and then pass
        // the unique results on to the scoring engine.
        // Returns true indicating the query evaluation should terminate 
        // (for example, due to early termination condition).
        // Returns false indicating the query evaluation should continue.
        // sliceBuffer represents the data buffer for the slice that the
        // matching engine is currently processing. There may be multiple
        // calls to FinishIteration() with the same sliceBuffer, but within
        // the iteration, sliceBuffer does not change.
        virtual bool FinishIteration(void const * sliceBuffer) = 0;

        // Returns true if the query execution was terminated early,
        // returns false otherwise.
        virtual bool TerminatedEarly() const = 0;
    };
}
