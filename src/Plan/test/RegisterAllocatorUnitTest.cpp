#include "stdafx.h"

#include "CompileNodes.h"
#include "MatchTreeRewriter.h"
#include "PrivateHeapAllocator.h"
#include "RankDownCompiler.h"
#include "BitFunnel/RowMatchNodes.h"
#include "RegisterAllocator.h"
#include "SuiteCpp/UnitTest.h"
#include "TextObjectFormatter.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace RegisterAllocatorUnitTest
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
                TestAssert(!registers.IsRegister(row));
            }
            else
            {
                TestAssert(registers.IsRegister(row));
                TestEqual(static_cast<unsigned>(expectedRegister),
                          registers.GetRegister(row));
            }
        }


        void VerifyCase(InputOutput const & testCase)
        {
            PrivateHeapAllocator allocator;
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


        TestCase(RegisterAllocation)
        {
            for (unsigned i = 0; i < sizeof(c_cases) / sizeof(InputOutput); ++i)
            {
                VerifyCase(c_cases[i]);
            }
        }
    }
}
