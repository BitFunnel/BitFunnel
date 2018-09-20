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

#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "CompileNode.h"
#include "MatchTreeCompiler.h"
#include "NativeCodeVerifier.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "RegisterAllocator.h"
#include "RowMatchNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    NativeCodeVerifier::NativeCodeVerifier(ISimpleIndex const & index,
                                           Rank initialRank)
      : CodeVerifierBase(index, initialRank)
    {
    }


    void NativeCodeVerifier::Verify(char const * codeText)
    {
        // Create allocator and buffers for pre-compiled and post-compiled code.
        NativeJIT::Allocator treeAllocator(8192);
        BitFunnel::Allocator allocator(2048);



        std::stringstream input(codeText);

        TextObjectParser parser(input, allocator, &CompileNode::GetType);
        CompileNode const & compileNodeTree = CompileNode::Parse(parser);

        RegisterAllocator registers(compileNodeTree,
                                    8,
                                    8,
                                    7,
                                    allocator);

        const size_t c_allocatorSize = 1ull << 17;
        auto treeallocator = std::make_unique<NativeJIT::Allocator>(c_allocatorSize);
        auto codeAllocator = std::make_unique<NativeJIT::ExecutionBuffer>(c_allocatorSize);
        auto code = std::make_unique<NativeJIT::FunctionBuffer>(*codeAllocator,
            static_cast<unsigned>(c_allocatorSize));

        MatchTreeCompiler compiler(*treeallocator,
                                   *code,
                                   compileNodeTree,
                                   registers,
                                   m_initialRank);

        ResultsBuffer results(m_index.GetIngestor().GetDocumentCount());

        compiler.Run(m_slices.size(),
                     m_slices.data(),
                     GetIterationsPerSlice(),
                     m_rowOffsets.data(),
                     results);

        CheckResults(results);
    }
}
