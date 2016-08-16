#include "gtest/gtest.h"

#include "CompileNodes.h"
#include "PlainTextCodeGenerator.h"
#include "Allocator.h"
#include "TextObjectFormatter.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace CompileNodeUnitTest
    {
        // TODO: Tests of illegal trees, e.g. and/or/phrases with 0, 1 child

        const char* c_parseFormatCases[] = {

            //
            // AndRowJz
            //

            // AndRowJz does not allow a null child.

            // AndRowJz followed by Report.
            "AndRowJz {\n"
            "  Row: Row(1, 2, 0, false),\n"
            "  Child: Report {\n"
            "    Child: \n"
            "  }\n"
            "}",

            //
            // LoadRowJz
            //

            // LoadRowJz with no child.
            "LoadRowJz {\n"
            "  Row: Row(1, 2, 0, false),\n"
            "  Child: Report {\n"
            "    Child: \n"
            "  }\n"
            "}",

            // LoadRowJz with Report child.
            "LoadRowJz {\n"
            "  Row: Row(1, 2, 0, false),\n"
            "  Child: Report {\n"
            "    Child: \n"
            "  }\n"
            "}",

            //
            // Or
            //

            // Or with 0 children no longer legal.
            // Or with 1 child no longer legal.

            // Or with two children.
            "Or {\n"
            "  Children: [\n"
            "    Report {\n"
            "      Child: \n"
            "    },\n"
            "    Report {\n"
            "      Child: \n"
            "    }\n"
            "  ]\n"
            "}",

            // Or with three children.
            "Or {\n"
            "  Children: [\n"
            "    Report {\n"
            "      Child: \n"
            "    },\n"
            "    Report {\n"
            "      Child: \n"
            "    },\n"
            "    Report {\n"
            "      Child: \n"
            "    }\n"
            "  ]\n"
            "}",

            //
            // RankDown
            //

            // RankDown with Report child.
            "RankDown {\n"
            "  Delta: 3,\n"
            "  Child: Report {\n"
            "    Child: \n"
            "  }\n"
            "}",

            //
            // Report
            //

            // Report with no child.
            "Report {\n"
            "  Child: \n"
            "}",

            // Report with LoadRowJz child.
            "Report {\n"
            "  Child: LoadRowJz {\n"
            "    Row: Row(1, 2, 0, false),\n"
            "    Child: Report {\n"
            "      Child: \n"
            "    }\n"
            "  }\n"
            "}",


            //
            // AndTree
            //
            "AndTree {\n"
            "  Children: [\n"
            "    LoadRow(0, 6, 0, false),\n"
            "    LoadRow(1, 0, 0, false)\n"
            "  ]\n"
            "}",

            "AndTree {\n"
            "  Children: [\n"
            "    LoadRow(0, 6, 0, false),\n"
            "    LoadRow(1, 3, 0, false),\n"
            "    LoadRow(2, 0, 0, false)\n"
            "  ]\n"
            "}",


            //
            // LoadRow
            //
            "LoadRow(0, 6, 0, false)",
            "LoadRow(1, 5, 0, true)",


            //
            // Not
            //
            "Not {\n"
            "  Child: LoadRow(0, 6, 0, false)\n"
            "}",

            //
            // OrTree
            //
            "OrTree {\n"
            "  Children: [\n"
            "    LoadRow(0, 6, 0, false),\n"
            "    LoadRow(1, 0, 0, false)\n"
            "  ]\n"
            "}",

            "OrTree {\n"
            "  Children: [\n"
            "    LoadRow(0, 6, 0, false),\n"
            "    LoadRow(1, 3, 0, false),\n"
            "    LoadRow(2, 0, 0, false)\n"
            "  ]\n"
            "}",
        };


        struct InputOutput
        {
        public:
            char const * m_input;
            char const * m_output;
        };


        const InputOutput c_codeGenerationCases[] =
        {
            // AndRowJz
            {
                "AndRowJz {\n"
                "  Row: Row(1, 2, 0, false),\n"
                "  Child: Report {\n"
                "    Child: \n"
                "  }\n"
                "}",
                "    AndRow(1, false, 0)\n"
                "    Jz(0)\n"
                "    Report()\n"
                "L0:\n"
            },

            // LoadRowJz
            {
                "LoadRowJz {\n"
                "  Row: Row(1, 2, 0, false),\n"
                "  Child: Report {\n"
                "    Child: \n"
                "  }\n"
                "}",
                "    LoadRow(1, false, 0)\n"
                "    Jz(0)\n"
                "    Report()\n"
                "L0:\n"
            },

            // Or
            {
                "Or {\n"
                "  Children: [\n"
                "    LoadRowJz {\n"
                "      Row: Row(0, 6, 0, false),\n"
                "      Child: Report {\n"
                "        Child: \n"
                "      }\n"
                "    },"
                "    LoadRowJz {\n"
                "      Row: Row(1, 6, 0, false),\n"
                "      Child: Report {\n"
                "        Child: \n"
                "      }\n"
                "    }"
                "  ]"
                "}",
                "    Push()\n"
                "    LoadRow(0, false, 0)\n"
                "    Jz(0)\n"
                "    Report()\n"
                "L0:\n"
                "    Pop()\n"
                "    LoadRow(1, false, 0)\n"
                "    Jz(1)\n"
                "    Report()\n"
                "L1:\n"
            },

            // RankDown
            {
                "RankDown {"
                "  Delta: 1,"
                "  Child: LoadRow(1, 2, 0, false)"
                "}",
                "    LeftShiftOffset(1)\n"
                "    Push()\n"
                "    Call(0)\n"
                "    Pop()\n"
                "    IncrementOffset()\n"
                "    Call(0)\n"
                "    Jmp(1)\n"
                "L0:\n"
                "    LoadRow(1, false, 0)\n"
                "    Return()\n"
                "L1:\n"
                "    RightShiftOffset(1)\n"
            },

            // Report with no child.
            // RankDown algorithm does Jz around Report().
            {
                "Report {"
                "  Child:"
                "}",
                "    Report()\n"
            },

            // Report with one child.
            // Includes Jz around Report().
            // TODO: What happens if this is the entire tree? Don't we need to
            // -1 into the accumulator first?
            {
                "Report {"
                "  Child: LoadRow(1, 2, 0, false)\n"
                "}",
                "    Push()\n"
                "    LoadRow(1, false, 0)\n"
                "    AndStack()\n"
                "    Jz(0)\n"
                "    Report()\n"
                "L0:\n"
            },

            {
                "AndTree {\n"
                "  Children: [\n"
                "    LoadRow(0, 6, 0, false),\n"
                "    LoadRow(1, 0, 0, false)\n"
                "  ]\n"
                "}",
                "    LoadRow(0, false, 0)\n"
                "    UpdateFlags()\n"
                "    Jz(0)\n"
                "    Push()\n"
                "    LoadRow(1, false, 0)\n"
                "    AndStack()\n"
                "L0:\n"
            },

            {
                "Not {\n"
                "  Child: LoadRow(0, 6, 0, false)\n"
                "}",
                "    LoadRow(0, false, 0)\n"
                "    Not()\n"
            },

            {
                "OrTree {\n"
                "  Children: [\n"
                "    LoadRow(0, 6, 0, false),\n"
                "    LoadRow(1, 0, 0, false)\n"
                "  ]\n"
                "}",
                "    LoadRow(0, false, 0)\n"
                "    Push()\n"
                "    LoadRow(1, false, 0)\n"
                "    OrStack()\n"
            }
        };


        //*********************************************************************
        //
        // Parse/Format cases.
        //
        //*********************************************************************
        void VerifyRoundtripCase(const char* text)
        {
            std::stringstream input(text);

            Allocator allocator(1024);
            TextObjectParser parser(input, allocator, &CompileNode::GetType);

            CompileNode const & node = CompileNode::Parse(parser);

            std::stringstream output;
            TextObjectFormatter formatter(output);
            node.Format(formatter);

            // std::cout << "\"" << text << "\"" << std::endl;
            // std::cout << "\"" << output.str() << "\"" << std::endl;

            EXPECT_EQ(text, output.str());
        }


        TEST(CompileNode, ParseFormat)
        {
            for (unsigned i = 0; i < sizeof(c_parseFormatCases) / sizeof(const char*); ++i)
            {
                VerifyRoundtripCase(c_parseFormatCases[i]);
            }
        }



        //*********************************************************************
        //
        // Code generation cases.
        //
        //*********************************************************************
        void VerifyCodeGenerationCase(InputOutput const & testCase)
        {
            std::stringstream input(testCase.m_input);

            Allocator allocator(1024);
            TextObjectParser parser(input, allocator, &CompileNode::GetType);

            CompileNode const & node = CompileNode::Parse(parser);


            std::stringstream output;
            PlainTextCodeGenerator code(output);
            node.Compile(code);

            //std::cout << "+++++++++++++++++++" << std::endl;
            //std::cout << "\"" << testCase.m_output << "\"" << std::endl;
            //std::cout << "+++" << std::endl;
            //std::cout << "\"" << output.str() << "\"" << std::endl;

            EXPECT_EQ(testCase.m_output, output.str());
        }


        TEST(CompileNode, CodeGeneration)
        {
            for (unsigned i = 0; i < sizeof(c_codeGenerationCases) / sizeof(InputOutput); ++i)
            {
                VerifyCodeGenerationCase(c_codeGenerationCases[i]);
            }
        }
    }
}
