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
#include "PlainTextCodeGenerator.h"
#include "RankDownCompiler.h"
#include "SameExceptForWhitespace.h"
#include "RowMatchNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace RankDownCompilerUnitTest
    {
        struct InputOutput
        {
        public:
            char const * m_input;
            char const * m_output;
        };


        const InputOutput c_cases[] =
        {
            ////
            //// Rows
            ////

            // Single rank 6 row.
            {
                "Row(0, 6, 0, false)",
                "LoadRowJz {"
                "  Row: Row(0, 6, 0, false),"
                "  Child: RankDown {"
                "    Delta: 6,"
                "    Child: Report {"
                "      Child: "
                "    }"
                "  }"
                "}"
            },

            // Single rank 6 row, inverted.
            {
                "Row(0, 6, 0, true)",
                "LoadRowJz {"
                "  Row: Row(0, 6, 0, true),"
                "  Child: RankDown {"
                "    Delta: 6,"
                "    Child: Report {"
                "      Child: "
                "    }"
                "  }"
                "}"
            },

            // Single, rank 0 row.
            // Expect RankDown to 0, then LoadRowJz, then Report.
            {
                "Row(0, 0, 0, false)",
                "RankDown {"
                "  Delta: 6,"
                "  Child: LoadRowJz {"
                "    Row: Row(0, 0, 0, false),"
                "    Child: Report {"
                "      Child: "
                "    }"
                "  }"
                "}"
            },

            // Single, rank 3 row.
            // Expect RankDown to 3, then LoadRowJz, then RankDown to 0, then Report.
            {
                "Row(0, 3, 0, false)",
                "RankDown {"
                "  Delta: 3,"
                "  Child: LoadRowJz {"
                "    Row: Row(0, 3, 0, false),"
                "    Child: RankDown {"
                "      Delta: 3,"
                "      Child: Report {"
                "        Child:"
                "      }"
                "    }"
                "  }"
                "}"
            },

            ////
            //// And
            ////

            // And two rank 6 rows.
            {
                "And {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Row(1, 6, 0, false)"
                "  ]"
                "}",
                "LoadRowJz {"
                "  Row: Row(0, 6, 0, false),"
                "  Child: AndRowJz {"
                "    Row: Row(1, 6, 0, false),"
                "    Child: RankDown {"
                "      Delta: 6,"
                "      Child: Report {"
                "        Child: "
                "      }"
                "    }"
                "  }"
                "}"
            },

            // And three rank 6 rows.
            // Expect LoadRowJz, followed by two AndRows, followed by a RankDown of 6.
            {
                "And {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Row(1, 6, 0, false),"
                "    Row(2, 6, 0, false)"
                "  ]"
                "}",
                "LoadRowJz {"
                "  Row: Row(0, 6, 0, false),"
                "  Child: AndRowJz {"
                "    Row: Row(1, 6, 0, false),"
                "    Child: AndRowJz {"
                "      Row: Row(2, 6, 0, false),"
                "      Child: RankDown {"
                "        Delta: 6,"
                "        Child: Report {"
                "          Child: "
                "        }"
                "      }"
                "    }"
                "  }"
                "}"
            },

            // And three rows with different ranks, first row is rank 6.
            // Should start with a LoadRowJz, followed by a RankDown of 3.
            {
                "And {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 0, 0, false)"
                "  ]"
                "}",
                "LoadRowJz {"
                "  Row: Row(0, 6, 0, false),"
                "  Child: RankDown {"
                "    Delta: 3,"
                "    Child: AndRowJz {"
                "      Row: Row(1, 3, 0, false),"
                "      Child: RankDown {"
                "        Delta: 3,"
                "        Child: AndRowJz {"
                "          Row: Row(2, 0, 0, false),"
                "          Child: Report {"
                "            Child: "
                "          }"
                "        }"
                "      }"
                "    }"
                "  }"
                "}"
            },


            // And three rows with different ranks, first row is rank 5.
            // Should start with a RankDown of 1, followed by a LoadRowJz.
            {
                "And {"
                "  Children: ["
                "    Row(0, 5, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 0, 0, false)"
                "  ]"
                "}",
                "RankDown {"
                "  Delta: 1,"
                "  Child: LoadRowJz {"
                "    Row: Row(0, 5, 0, false),"
                "    Child: RankDown {"
                "      Delta: 2,"
                "      Child: AndRowJz {"
                "        Row: Row(1, 3, 0, false),"
                "        Child: RankDown {"
                "          Delta: 3,"
                "          Child: AndRowJz {"
                "            Row: Row(2, 0, 0, false),"
                "            Child: Report {"
                "              Child: "
                "            }"
                "          }"
                "        }"
                "      }"
                "    }"
                "  }"
                "}"
            },


            // TODO: This test should be reenabled once out of order loads have been
            // implemented. And three rows with non-descending ranks.
            // Should start with a RankDown of 1, followed by a LoadRowJzs and
            // then an AndRowJz with RankUp by 1. Next is a RankDown of 4
            // followed by a final AndRowJz with RankUp of 0.
            {
                "And {"
                "  Children: ["
                "    Row(0, 5, 0, false),"
                "    Row(1, 5, 1, false),"
                "    Row(2, 0, 0, false)"
                "  ]"
                "}",
                "RankDown {"
                "  Delta: 1,"
                "  Child: LoadRowJz {"
                "    Row: Row(0, 5, 0, false),"
                "    Child: AndRowJz {"
                "      Row: Row(1, 5, 1, false),"
                "      Child: RankDown {"
                "        Delta: 5,"
                "        Child: AndRowJz {"
                "          Row: Row(2, 0, 0, false),"
                "          Child: Report {"
                "            Child: "
                "          }"
                "        }"
                "      }"
                "    }"
                "  }"
                "}"
            },


            //
            // Or
            //

            // Or two rank 6 rows.
            // Expect each LoadRow followed by RankDown 6 then report.
            {
                "Or {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Row(1, 6, 0, false)"
                "  ]"
                "}",
                "Or {"
                "  Children: ["
                "    LoadRowJz {"
                "      Row: Row(0, 6, 0, false),"
                "      Child: RankDown {"
                "        Delta: 6,"
                "        Child: Report {"
                "          Child: "
                "        }"
                "      }"
                "    },"
                "    LoadRowJz {"
                "      Row: Row(1, 6, 0, false),"
                "      Child: RankDown {"
                "        Delta: 6,"
                "        Child: Report {"
                "          Child: "
                "        }"
                "      }"
                "    }"
                "  ]"
                "}"
            },

            // Or a rank 5 row with a rank 3 row.
            // Goal is to test RankDowns interspersed with other primitives.
            // Expect Rankdown 1 followed by Or.
            // First branh of Or is LoadRow 0, followed by RankDown 5 and report.
            // Second branch is RankDown 2, LoadRow 1, RankDown 3.
            {
                "Or {"
                "  Children: ["
                "    Row(0, 5, 0, false),"
                "    Row(1, 3, 0, false)"
                "  ]"
                "}",
                "RankDown {"
                "  Delta: 1,"
                "  Child: Or {"
                "    Children: ["
                "      LoadRowJz {"
                "        Row: Row(0, 5, 0, false),"
                "        Child: RankDown {"
                "          Delta: 5,"
                "          Child: Report {"
                "            Child: "
                "          }"
                "        }"
                "      },"
                "      RankDown {"
                "        Delta: 2,"
                "        Child: LoadRowJz {"
                "          Row: Row(1, 3, 0, false),"
                "          Child: RankDown {"
                "            Delta: 3,"
                "            Child: Report {"
                "              Child: "
                "            }"
                "          }"
                "        }"
                "      }"
                "    ]"
                "  }"
                "}"
            },

            // Or three rank 6 rows.
            // Goal is to validate tree flattening code by processing a tree
            // of three children which is represented by two binary tree.
            {
                "Or {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Row(1, 6, 0, false),"
                "    Row(2, 6, 0, false)"
                "  ]"
                "}",
                "Or {"
                "  Children: ["
                "    LoadRowJz {"
                "      Row: Row(0, 6, 0, false),"
                "      Child: RankDown {"
                "        Delta: 6,"
                "        Child: Report {"
                "          Child: "
                "        }"
                "      }"
                "    },"
                "    LoadRowJz {"
                "      Row: Row(1, 6, 0, false),"
                "      Child: RankDown {"
                "        Delta: 6,"
                "        Child: Report {"
                "          Child: "
                "        }"
                "      }"
                "    },"
                "    LoadRowJz {"
                "      Row: Row(2, 6, 0, false),"
                "      Child: RankDown {"
                "        Delta: 6,"
                "        Child: Report {"
                "          Child: "
                "        }"
                "      }"
                "    }"
                "  ]"
                "}"
            },

            // Or three rows with different ranks.
            {
                "Or {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 0, 0, false)"
                "  ]"
                "}",
                "Or {"
                "  Children: ["
                "    LoadRowJz {"
                "      Row: Row(0, 6, 0, false),"
                "      Child: RankDown {"
                "        Delta: 6,"
                "        Child: Report {"
                "          Child: "
                "        }"
                "      }"
                "    },"
                "    RankDown {"
                "      Delta: 3,"
                "      Child: Or {"
                "        Children: ["
                "          LoadRowJz {"
                "            Row: Row(1, 3, 0, false),"
                "            Child: RankDown {"
                "              Delta: 3,"
                "              Child: Report {"
                "                Child: "
                "              }"
                "            }"
                "          },"
                "          RankDown {"
                "            Delta: 3,"
                "            Child:LoadRowJz {"
                "              Row: Row(2, 0, 0, false),"
                "              Child: Report {"
                "                Child: "
                "              }"
                "            }"
                "          }"
                "        ]"
                "      }"
                "    }"
                "  ]"
                "}"
            },

            // Empty Report.
            {
                "Report {"
                "  Child: "
                "}",
                "RankDown {"
                "  Delta: 6,"
                "  Child: Report {"
                "    Child: "
                "  }"
                "}"
            },

            // Report with LoadRow, rank 0.
            // Expect RankDown 6 before LoadRow.
            {
                "Report {"
                "  Child: Row(0, 0, 0, false)"
                "}",
                "RankDown {"
                "  Delta: 6,"
                "  Child: Report {"
                "    Child: LoadRow(0, 0, 0, false)"
                "  }"
                "}"
            },

            // Report with LoadRow, rank 3.
            // Still expect RankDown 6 before LoadRow.
            {
                "Report {"
                "  Child: Row(0, 3, 0, false)"
                "}",
                "RankDown {"
                "  Delta: 6,"
                "  Child: Report {"
                "    Child: LoadRow(0, 3, 0, false)"
                "  }"
                "}"
            },


            //
            // Combinations
            //

            // And one rank 6 row with Or of two rank 6 rows.
            // Goal is to verify correct selection of LoadRow vs AndRow.
            {
                "And {"
                "  Children: ["
                "    Row(0, 6, 0, false),"
                "    Or {"
                "      Children: ["
                "        Row(1, 6, 0, false),"
                "        Row(2, 6, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}",
                "LoadRowJz {"
                "  Row: Row(0, 6, 0, false),"
                "  Child: Or {"
                "    Children: ["
                "      AndRowJz {"
                "        Row: Row(1, 6, 0, false),"
                "        Child: RankDown {"
                "          Delta: 6,"
                "          Child: Report {"
                "            Child: "
                "          }"
                "        }"
                "      },"
                "      AndRowJz {"
                "        Row: Row(2, 6, 0, false),"
                "        Child: RankDown {"
                "          Delta: 6,"
                "          Child: Report {"
                "            Child: "
                "          }"
                "        }"
                "      }"
                "    ]"
                "  }"
                "}"
            },
        };


        // void PrettyPrint(const char * plan)
        // {
        //     std::stringstream input(plan);
        //     PrivateHeapAllocator allocator;
        //     TextObjectParser parser(input, allocator, &CompileNode::GetType);
        //     CompileNode const & node = CompileNode::Parse(parser);

        //     std::stringstream output;
        //     TextObjectFormatter formatter(output);
        //     node.Format(formatter);

        //     std::cout << output.str() << std::endl;
        // }


        void VerifyCase(InputOutput const & testCase)
        {
            std::stringstream input(testCase.m_input);

            Allocator allocator(512);
            TextObjectParser parser(input, allocator, &RowPlanBase::GetType);
            RowMatchNode const & root = RowMatchNode::Parse(parser);

            //std::stringstream output;
            //PlainTextCodeGenerator printer(output);
            //RankDownCompiler(6, root, printer);

            RankDownCompiler compiler(allocator);
            compiler.Compile(root);
            CompileNode const & compiled = compiler.CreateTree(6);

            std::stringstream output;
            TextObjectFormatter formatter2(output);
            compiled.Format(formatter2);

            //if (!SameExceptForWhitespace(output.str().c_str(), testCase.m_output))
            //{
            //    std::cout << "===================" << std::endl;
            //    PrettyPrint(testCase.m_output);
            //    std::cout << "===" << std::endl;
            //    std::cout << output.str() << std::endl;
            //}


            EXPECT_TRUE(SameExceptForWhitespace(output.str().c_str(), testCase.m_output));
        }


        TEST(RankDownCompiler,Compile)
        {
            for (unsigned i = 0; i < sizeof(c_cases) / sizeof(InputOutput); ++i)
            {
                VerifyCase(c_cases[i]);
            }
        }
    }
}
