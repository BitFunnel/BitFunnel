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
#include "RowSet.h"


namespace BitFunnel
{
    class IPlanRows;
    class ISimpleIndex;
    class IThreadResources;
    class QueryInstrumentation;
    class RowSet;
    class TermMatchNode;

    class QueryPlanner : public NonCopyable
    {
    public:
        // Constructs a QueryPlanner with the specified resources.
        QueryPlanner(TermMatchNode const & tree,
                     unsigned targetRowCount,
                     ISimpleIndex const & index,
                     IAllocator & matchTreeAllocator,
                     IDiagnosticStream& diagnosticStream,
                     QueryInstrumentation & instrumentation);

        CompileNode const & GetCompileTree() const;

        Rank GetInitialRank() const;

        RowSet const & GetRowSet() const;

        IPlanRows const & GetPlanRows() const;

    private:
        CompileNode const * m_compileTree;

        Rank m_initialRank;

        std::unique_ptr<RowSet> m_rowSet;
        
        IPlanRows const * m_planRows;

    };
}
