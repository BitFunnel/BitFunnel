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

#include "gtest/gtest.h"

#include "Allocator.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Mocks/Factories.h"
#include "ByteCodeInterpreter.h"
#include "CompileNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    size_t c_allocatorBufferSize = 1000000;

    void GenerateCode(char const * rowPlanText,
                      ByteCodeGenerator& code)
    {
        std::stringstream rowPlan(rowPlanText);

        Allocator allocator(c_allocatorBufferSize);
        TextObjectParser parser(rowPlan, allocator, &CompileNode::GetType);

        CompileNode const & node = CompileNode::Parse(parser);

        node.Compile(code);
    }


    void RunTest()
    {
        const DocId maxDocId = 63;
        const Term::StreamId streamId = 0;

        const size_t maxGramSize = 1;

        auto fileSystem = Factories::CreateRAMFileSystem();

        auto index =
            Factories::CreatePrimeFactorsIndex(*fileSystem,
                                               maxDocId,
                                               streamId);
        //index->GetIngestor().GetShard();
    }


    TEST(ByteCodeInterpreter, Placeholder)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"      // Row(0) is 0, 1, 2, ...
            "  Child: AndRowJz {"
            "    Row: Row(2, 0, 0, false),"    // Row(2) is AAAAAAA....
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        ByteCodeGenerator code;
        GenerateCode(text, code);
    }
}
