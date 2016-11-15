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

#include <sstream>
#include <vector>

#include "gtest/gtest.h"

#include "Allocator.h"
#include "BitFunnel/Plan/RowMatchNode.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "MatchTreeCompiler.h"
#include "MatchTreeRewriter.h"
#include "NativeCodeGenerator.h"
#include "RankDownCompiler.h"
#include "RegisterAllocator.h"
#include "Temporary/Allocator.h"
#include "TextObjectParser.h"

using namespace NativeJIT;


namespace BitFunnel
{
    class MockSlice
    {
    public:
        MockSlice(size_t iterations, size_t rowCount);

        char * GetSliceBuffer() const;
        ptrdiff_t GetRowOffset(size_t rowIndex) const;

    private:
        const size_t m_iterations;
        const size_t m_rowCount;
        std::unique_ptr<char[]> m_buffer;
        std::vector<int> m_values;
    };


    MockSlice::MockSlice(size_t iterations, size_t rowCount)
      : m_iterations(iterations),
        m_rowCount(rowCount),
        m_buffer(new char[m_iterations * m_rowCount * 8]),
        m_values({ 0xff, 0xaa, 0x88, 0x80 })
    {
        char *p = m_buffer.get();
        for (size_t r = 0; r < m_rowCount; ++r)
        {
            char value = static_cast<char>(m_values[r]);
            for (size_t i = 0; i < m_iterations; ++i)
            {
                for (size_t j = 0; j < 8; ++j)
                {
                    if ((r == m_rowCount - 1) && ((j % 8) != 0))
                    {
                        *p++ = 0;
                    }
                    else
                    {
                        *p++ = value;
                    }
                }
            }
        }
    }


    char * MockSlice::GetSliceBuffer() const
    {
        return m_buffer.get();
    }


    ptrdiff_t MockSlice::GetRowOffset(size_t rowIndex) const
    {
        return m_iterations * 8 * rowIndex;
    }



    TEST(MatchTreeCompiler, Placeholder)
    {
        // Create allocator and buffers for pre-compiled and post-compiled code.
        NativeJIT::ExecutionBuffer codeAllocator(8192);
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
            "    Row(0, 0, 0, false),"
            "    Row(1, 0, 0, false),"
            "    Row(2, 0, 0, false)"
            "  ]"
            "}");


        TextObjectParser parser(input, allocator, &RowMatchNode::GetType);
        RowMatchNode const & node = RowMatchNode::Parse(parser);

        RowMatchNode const & rewritten = MatchTreeRewriter::Rewrite(node, 0, 20, allocator);

        RankDownCompiler rankDown(allocator);
        rankDown.Compile(rewritten);
        CompileNode const & compileNodeTree = rankDown.CreateTree(0);

        // Use register base value of 100 in unit tests to allow
        // easy verification of the base value.
        RegisterAllocator registers(compileNodeTree,
                                    7,
                                    8,
                                    7,
                                    allocator);


        MatchTreeCompiler compiler(codeAllocator,
                                   treeAllocator,
                                   compileNodeTree,
                                   registers);

        const size_t iterationCount = 2;
        const size_t rowCount = 3;

        MockSlice slice0(iterationCount, rowCount);
        MockSlice slice1(iterationCount, rowCount);
        MockSlice slice2(iterationCount, rowCount);

        std::vector<void *> sliceBuffers;
        sliceBuffers.push_back(slice0.GetSliceBuffer());
        sliceBuffers.push_back(slice1.GetSliceBuffer());
        sliceBuffers.push_back(slice2.GetSliceBuffer());

        std::vector<ptrdiff_t> rowOffsets;
        rowOffsets.push_back(slice0.GetRowOffset(0));
        rowOffsets.push_back(slice0.GetRowOffset(1));
        rowOffsets.push_back(slice0.GetRowOffset(2));

        const size_t matchCapacity =
            iterationCount * sliceBuffers.size() * 64;

        std::vector<NativeCodeGenerator::Record>
            matches(matchCapacity, { nullptr, 0 });


        // TODO: return result of compiler.Run() once implemented.
        compiler.Run(sliceBuffers.size(),
                     sliceBuffers.data(),
                     iterationCount,
                     rowOffsets.data(),
                     matchCapacity,
                     matches.data());
    }
}
