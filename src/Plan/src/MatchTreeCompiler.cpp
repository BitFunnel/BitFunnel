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


#include "BitFunnel/Utilities/Allocator.h"
#include "MatchTreeCompiler.h"

using namespace NativeJIT;


namespace BitFunnel
{
    //*************************************************************************
    //
    // MatchTreeCompiler
    //
    //*************************************************************************
    MatchTreeCompiler::MatchTreeCompiler(Allocators::IAllocator & expressionTreeAllocator,
                                         NativeJIT::FunctionBuffer & code,
                                         CompileNode const & tree,
                                         RegisterAllocator const & registers,
                                         Rank initialRank)
    {
        NativeCodeGenerator::Prototype expression(expressionTreeAllocator,
                                                  code);
        // TODO: Remove temporary debugging output.
        //expression.EnableDiagnostics(std::cout);

        auto & node =
            expression.PlacementConstruct<NativeCodeGenerator>(expression,
                                                               tree,
                                                               registers,
                                                               initialRank);
        m_function = expression.Compile(node);
    }


    size_t MatchTreeCompiler::Run(size_t sliceCount,
                                  void * const * sliceBuffers,
                                  size_t iterationsPerSlice,
                                  ptrdiff_t const * rowOffsets,
                                  ResultsBuffer & results)
    {
        NativeCodeGenerator::Parameters parameters = {
            sliceCount,
            sliceBuffers,
            iterationsPerSlice,
            rowOffsets,
            0,
            { 0 },
            results.m_capacity,
            results.m_size,
            results.m_buffer,
            0
        };

        // For now ignore return value.
        m_function(&parameters);

        // TODO: Remove temporary debugging output.
        //std::cout
        //    << parameters.m_matchCount
        //    << " matches."
        //    << std::endl;

        //for (size_t i = 0; i < parameters.m_matchCount; ++i)
        //{
        //    auto & match = parameters.m_matches[i];
        //    std::cout
        //        << std::hex
        //        << match.m_buffer
        //        << std::dec
        //        << ": "
        //        << match.m_id
        //        << std::endl;
        //}

        results.m_size = parameters.m_matchCount;

        return parameters.m_quadwordCount;
    }
}
