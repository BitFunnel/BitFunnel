// The MIT License (MIT)

// Copyright (c) 2018, Microsoft

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

#include "BitFunnel/IInterface.h"       // Base class.


namespace BitFunnel
{
    class QueryInstrumentation;
    class ResultsBuffer;
    class TermMatchNode;

    //*************************************************************************
    //
    // IQueryEngine
    //
    // An abstract base class or interface for concrete QueryEngine classes,
    // such as thuse that use NativeJIT or interpret the query tree.
    // It defines the common public methods needed to run parsed queries.
    //
    //*************************************************************************
    class IQueryEngine : public IInterface
    {
    public:
        // Parse a query
        virtual TermMatchNode const *Parse(const char *query) = 0;

        // Runs a parsed query
        virtual void Run(TermMatchNode const * tree,
                         QueryInstrumentation & instrumentation,
                         ResultsBuffer & resultsBuffer) = 0;

        // Adds the diagnostic keyword prefix to the list of prefixes that
        // enable diagnostics.
        virtual void EnableDiagnostic(char const * prefix) = 0;

        // Removes the diagnostic keyword prefix from the list of prefixes
        // that enable diagnostics.
        virtual void DisableDiagnostic(char const * prefix) = 0;

    };
}
