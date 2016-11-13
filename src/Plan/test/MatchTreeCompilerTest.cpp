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

#include <iostream>
#include <sstream>

#include "gtest/gtest.h"

#include "Allocator.h"
#include "BitFunnel/Plan/RowMatchNode.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "MatchTreeCodeGenerator.h"
#include "MatchTreeRewriter.h"
#include "RankDownCompiler.h"
#include "RegisterAllocator.h"
#include "Temporary/Allocator.h"
#include "TextObjectParser.h"

using namespace NativeJIT;


namespace BitFunnel
{
    TEST(MatchTreeCompiler, Placeholder)
    {
        // Temporarily disabling test code below.
    }

    void TestUnderDevelopment()
    {
        // Create allocator and buffers for pre-compiled and post-compiled code.
        ExecutionBuffer codeAllocator(8192);
        NativeJIT::Allocator treeAllocator(8192);
        BitFunnel::Allocator allocator(2048);

        //std::stringstream input(
        //    "And {"
        //    "  Children: ["
        //    "    Row(0, 0, 0, false),"
        //    "    Row(1, 3, 0, false),"
        //    "    Row(2, 6, 0, false),"
        //    "    Or {"
        //    "      Children: ["
        //    "        Row(4, 3, 0, false),"
        //    "        Row(3, 0, 0, false)"
        //    "      ]"
        //    "    },"
        //    "    Or {"
        //    "      Children: ["
        //    "        Row(5, 0, 0, false),"
        //    "        Row(6, 3, 0, false)"
        //    "      ]"
        //    "    }"
        //    "  ]"
        //    "}");

        std::stringstream input(
            "And {"
            "  Children: ["
            "    Row(0, 6, 0, false),"
            "    Row(1, 3, 0, false),"
            "    Row(2, 0, 0, false)"
            "  ]"
            "}");


        TextObjectParser parser(input, allocator, &RowMatchNode::GetType);
        RowMatchNode const & node = RowMatchNode::Parse(parser);

        RowMatchNode const & rewritten = MatchTreeRewriter::Rewrite(node, 0, 20, allocator);

        RankDownCompiler rankDown(allocator);
        rankDown.Compile(rewritten);
        CompileNode const & compileNodeTree = rankDown.CreateTree(6);

        // Use register base value of 100 in unit tests to allow
        // easy verification of the base value.
        RegisterAllocator registers(compileNodeTree,
                                    7,
                                    8,
                                    7,
                                    allocator);



        //CompileNode const * compileNodeTree = nullptr;
        //RegisterAllocator registers;

        MatchTreeCompiler compiler(codeAllocator,
                                   treeAllocator,
                                   compileNodeTree,
                                   registers);

        std::vector<char *> m_sliceBuffers(3, 0);
        std::vector<ptrdiff_t> m_rowOffsets(2, 0);

        auto result = compiler.Run(m_sliceBuffers.size(),
                                   m_sliceBuffers.data(),
                                   80,                          // Bytes per slice at max rank.
                                   m_rowOffsets.data());

        std::cout << "Result = " << result << std::endl;
    }
}
