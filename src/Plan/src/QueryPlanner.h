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

#include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.
#include "ByteCodeInterpreter.h"


namespace BitFunnel
{
    class IPlanRows;
    class ISimpleIndex;
    class IThreadResources;
    class QueryInstrumentation;
    class QueryResources;
    class ResultsBuffer;
    class RowSet;
    class TermMatchNode;

    class QueryPlanner : public NonCopyable
    {
    public:
        // Constructs a QueryPlanner with the specified resources.
        QueryPlanner(TermMatchNode const & tree,
                     unsigned targetRowCount,
                     ISimpleIndex const & index,
                     // IThreadResources& threadResources,
                     QueryResources & resources,
                     IDiagnosticStream& diagnosticStream,
                     QueryInstrumentation & instrumentation,
                     ResultsBuffer & resultsBuffer,
                     bool useNativeCode);

        IPlanRows const & GetPlanRows() const;

    private:
        void RunByteCodeInterpreter(ISimpleIndex const & index,
                                    QueryResources & resources,
                                    QueryInstrumentation & instrumentation,
                                    CompileNode const & compileTree,
                                    Rank maxRank,
                                    RowSet const & rowSet);

        void RunNativeCode(ISimpleIndex const & index,
                           QueryResources & resources,
                           QueryInstrumentation & instrumentation,
                           CompileNode const & compileTree,
                           Rank maxRank,
                           RowSet const & rowSet);

        IPlanRows const * m_planRows;

        // The maximum number of iterations that can be performed before a termination
        // check is mandatory. Details can be found in the MatchTreeCodeGenerator.
        // const unsigned m_maxIterationsScannedBetweenTerminationChecks;

        // First available row pointer register is R8.
        // TODO: is this valid on all platforms or only on Windows?
        static const unsigned c_registerBase = 8;

        // Row pointers stored in the eight registers R8..R15.
        // TODO: is this valid on all platforms or only on Windows?
        static const unsigned c_registerCount = 8;

        ByteCodeGenerator m_code;

        ResultsBuffer& m_resultsBuffer;
    };
}
