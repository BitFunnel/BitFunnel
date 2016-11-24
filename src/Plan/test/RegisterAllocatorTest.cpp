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

#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/TextObjectFormatter.h"
#include "CompileNode.h"
#include "MatchTreeRewriter.h"
#include "RankDownCompiler.h"
#include "RegisterAllocator.h"
#include "RowMatchNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace RegisterAllocatorTest
    {
        struct InputOutput
        {
            const char * m_input;
            unsigned m_rowCount;
            unsigned m_registerCount;
            int m_registers[100];
        };


        InputOutput const c_cases[] =
        {
            // Test Not, AndTree, OrTree, and LoadRow.
            {
                "Not {"
                "   Child: And {"
                "     Children: ["
                "       Row(2, 0, 0, false),"
                "       Or {"
                "         Children: ["
                "           Row(0, 0, 0, false),"
                "           Row(1, 0, 0, false)"
                "         ]"
                "       }"
                "     ]"
                "   }"
                "}",
                3,
                3,
                {
                    101,
                    102,
                    100
                }
            },


            {
                // Moderateley complex example with LoadRowJz, AndRowJz,
                // RankDown, Or, and Report.
                "And {"
                "  Children: ["
                "    Row(0, 0, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 6, 0, false),"
                "    Or {"
                "      Children: ["
                "        Row(4, 3, 0, false),"
                "        Row(3, 0, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(5, 0, 0, false),"
                "        Row(6, 3, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}",
                7,
                7,
                {
                    106,    // Row 0
                    101,    // Row 1
                    100,    // Row 2
                    102,    // Row 3
                    103,    // Row 4
                    105,    // Row 5 
                    104,    // Row 6
                }
            },


            // Same as above, but order of rank 0 rows inside of Or node has
            // been revered. Expect the same register allocation. When two rows
            // are at the same depth, the row that is used more (usually it is
            // under a RankDown) has priority for register allocation.
            {
                "And {"
                "  Children: ["
                "    Row(0, 0, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 6, 0, false),"
                "    Or {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(4, 3, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(5, 0, 0, false),"
                "        Row(6, 3, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}",
                7,
                7,
                {
                    106,    // Row 0
                    101,    // Row 1
                    100,    // Row 2
                    102,    // Row 3
                    103,    // Row 4
                    105,    // Row 5
                    104,    // Row 6
                }
            },


            // Same as above, but register count is set to 4, even though
            // there are 7 rows.
            {
                "And {"
                "  Children: ["
                "    Row(0, 0, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 6, 0, false),"
                "    Or {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(4, 3, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(5, 0, 0, false),"
                "        Row(6, 3, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}",
                7,
                4,
                {
                    -1,    // Row 0
                    101,    // Row 1
                    100,    // Row 2
                    102,    // Row 3
                    103,    // Row 4
                    -1,    // Row 5
                    -1,    // Row 6
                }
            },


            // Same as above, but register count is set to 4, even though
            // there are 7 rows.
            {
                "And {"
                "  Children: ["
                "    Row(0, 0, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 6, 0, false),"
                "    Or {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(4, 3, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(5, 0, 0, false),"
                "        Row(6, 3, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}",
                7,
                0,
                {
                    -1,    // Row 0
                    -1,    // Row 1
                    -1,    // Row 2
                    -1,    // Row 3
                    -1,    // Row 4
                    -1,    // Row 5
                    -1,    // Row 6
                }
            },


            // Verify that rows at the same depth are ordered by usage.
            // In this test, row 1 should be in the first register because it
            // is used 64 times after the rankdown, while row 0 is only used
            // once.
            {
                "Or {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Row(1, 0, 0, false)"
                "  ]"
                "}",
                2,
                2,
                {
                    101,    // Row 0
                    100,    // Row 1
                }
            }
        };


        void VerifyRegister(unsigned row,
                            int expectedRegister,
                            RegisterAllocator const & registers)
        {
            if (expectedRegister < 0)
            {
                EXPECT_FALSE(registers.IsRegister(row));
            }
            else
            {
                EXPECT_TRUE(registers.IsRegister(row));
                EXPECT_EQ(static_cast<unsigned>(expectedRegister),
                          registers.GetRegister(row));
            }
        }


        void VerifyCase(InputOutput const & testCase, IAllocator & allocator)
        {
            std::stringstream input(testCase.m_input);
            TextObjectParser parser(input, allocator, &RowMatchNode::GetType);
            RowMatchNode const & node = RowMatchNode::Parse(parser);

            RowMatchNode const & rewritten = MatchTreeRewriter::Rewrite(node, 6, 20, allocator);

            RankDownCompiler rankDown(allocator);
            rankDown.Compile(rewritten);
            CompileNode const & compiled = rankDown.CreateTree(6);

            // Use register base value of 100 in unit tests to allow
            // easy verification of the base value.
            RegisterAllocator registers(compiled,
                                        testCase.m_rowCount,
                                        100,
                                        testCase.m_registerCount,
                                        allocator);

            for (unsigned i = 0; i < testCase.m_rowCount; ++i)
            {
                VerifyRegister(i, testCase.m_registers[i], registers);
            }
        }


        TEST(RegisterAllocator,Basic)
        {
            Allocator allocator(2048);
            for (unsigned i = 0; i < sizeof(c_cases) / sizeof(InputOutput); ++i)
            {
                VerifyCase(c_cases[i], allocator);
                allocator.Reset();
            }
        }
    }
}
