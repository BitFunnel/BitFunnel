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

#include "BitFunnel/IDiagnosticStream.h" // TODO: remove.
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/Factories.h"  // TODO: only for diagnosticStream. Remove.
#include "ByteCodeInterpreter.h"
#include "ByteCodeVerifier.h"
#include "CompileNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    static const size_t c_allocatorBufferSize = 1000000;


    ByteCodeVerifier::ByteCodeVerifier(ISimpleIndex const & index,
                                       Rank initialRank)
      : CodeVerifierBase(index, initialRank)
    {
    }


    void ByteCodeVerifier::Verify(char const * codeText)
    {
        Allocator allocator(c_allocatorBufferSize);
        ByteCodeGenerator code;

        std::stringstream input(codeText);

        TextObjectParser parser(input, allocator, &CompileNode::GetType);
        CompileNode const & node = CompileNode::Parse(parser);

        node.Compile(code);
        code.Seal();

        QueryInstrumentation instrumentation;

        ResultsBuffer results(m_index.GetIngestor().GetDocumentCount());
        ByteCodeInterpreter interpreter(
            code,
            results,
            m_slices.size(),
            m_slices.data(),
            GetIterationsPerSlice(),
            m_initialRank,
            m_rowOffsets.data(),
            nullptr,
            instrumentation,
            0);

        interpreter.Run();

        CheckResults(results);
    }
}
