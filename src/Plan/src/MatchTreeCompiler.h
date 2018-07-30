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

#include <stddef.h>                             // size_t, ptrdiff_t parameters.

#include "BitFunnel/BitFunnelTypes.h"           // Rank parameter.
#include "NativeCodeGenerator.h"                // MatcherNode::Prototype::FunctionType type.
#include "NativeJIT/CodeGen/FunctionBuffer.h"   // FunctionBuffer embedded.


namespace NativeJIT
{
    class Allocator;
    class ExecutionBuffer;
}; 


namespace BitFunnel
{
    class CompileNode;
    class RegisterAllocator;
    class ResultsBuffer;

    //*************************************************************************
    //
    // MatchTreeCompiler
    //
    //*************************************************************************
    class MatchTreeCompiler
    {
    public:
        MatchTreeCompiler(Allocators::IAllocator & resources,
                          NativeJIT::FunctionBuffer & code,
                          CompileNode const & tree,
                          RegisterAllocator const & registers,
                          Rank initialRank);

        size_t Run(size_t slicecount,
                   void * const * slicebuffers,
                   size_t iterationsperslice,
                   ptrdiff_t const * rowoffsets,
                   ResultsBuffer & results);

    private:
        NativeCodeGenerator::Prototype::FunctionType m_function;
    };
}
