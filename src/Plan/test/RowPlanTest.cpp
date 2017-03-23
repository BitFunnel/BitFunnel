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
#include "RowMatchNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace RowPlanTest
    {
        // TODO: Tests of illegal trees, e.g. and/or/phrases with 0, 1 child

        char const * c_rowMatchNodeCases[] = {

            //
            // RowMatchNode::And
            //

            // And with 0 children no longer legal.
            // And with 1 child no longer legal.

            // And node with two children
            "And {\n"
            "  Children: [\n"
            "    Row(1, 2, 0, false),\n"
            "    Row(3, 4, 0, false)\n"
            "  ]\n"
            "}",

            // And node with three children
            "And {\n"
            "  Children: [\n"
            "    Row(1, 2, 0, false),\n"
            "    Row(3, 4, 0, false),\n"
            "    Row(5, 6, 0, false)\n"
            "  ]\n"
            "}",


            //
            // RowMatchNode::Or
            //

            // Or with 0 children no longer legal.
            // Or with 1 child no longer legal.

            // Or node with two children
            "Or {\n"
            "  Children: [\n"
            "    Row(1, 2, 0, false),\n"
            "    Row(3, 4, 0, false)\n"
            "  ]\n"
            "}",

            // Or node with three children
            "Or {\n"
            "  Children: [\n"
            "    Row(1, 2, 0, false),\n"
            "    Row(3, 4, 0, false),\n"
            "    Row(5, 6, 0, false)\n"
            "  ]\n"
            "}",


            //
            // RowMatchNode::Not
            //

            // Not node
            "Not {\n"
            "  Child: Row(1, 1, 0, false)\n"
            "}",


            //
            // RowMatchNode::Report
            //

            // Report node with null child.
            "Report {\n"
            "  Child: \n"
            "}",

            // Report node with Row child.
            "Report {\n"
            "  Child: Row(1, 1, 0, false)\n"
            "}",


            //
            // RowMatchNode:Row
            //

            // Leaf nodes
            "Row(1, 2, 0, false)",
            "Row(3, 4, 0, true)",

            // Leaf nodes with non-zero rank delta.
            "Row(1, 2, 1, false)",
            "Row(3, 4, 2, true)",


            //
            // Other cases
            //

            // Complex tree where And and Or nodes each have multiple children.
            "And {\n"
            "  Children: [\n"
            "    Or {\n"
            "      Children: [\n"
            "        Row(0, 1, 0, false),\n"
            "        Row(3, 2, 0, false)\n"
            "      ]\n"
            "    },\n"
            "    Not {\n"
            "      Child: And {\n"
            "        Children: [\n"
            "          Row(6, 3, 0, false),\n"
            "          Row(6, 4, 0, false)\n"
            "        ]\n"
            "      }\n"
            "    }\n"
            "  ]\n"
            "}",
        };


        // const char* c_rowPlanCases[] = {
        //     //
        //     // RowPlan
        //     //
        //     "RowPlan {\n"
        //     "  Match: Row(0, 1, 0, false)\n"
        //     "}",
        // };


        template <class T>
        void VerifyRoundtripCase(const char* text)
        {
            std::stringstream input(text);

            Allocator allocator(256);
            TextObjectParser parser(input, allocator, &RowPlanBase::GetType);

            T const & node = T::Parse(parser);

            std::stringstream output;
            TextObjectFormatter formatter(output);
            node.Format(formatter);

            //std::cout << "\"" << text << "\"" << std::endl;
            //std::cout << "\"" << output.str() << "\"" << std::endl;

            EXPECT_STREQ(text, output.str().c_str());
        }


        //*********************************************************************
        //
        // Parse/Format cases.
        //
        //*********************************************************************
        TEST(RowPlan,RowMatchNode)
        {
            for (unsigned i = 0; i < sizeof(c_rowMatchNodeCases) / sizeof(const char*); ++i)
            {
                VerifyRoundtripCase<RowMatchNode>(c_rowMatchNodeCases[i]);
            }
        }


        // TEST(RowPlan,RowPlan)
        // {
        //     for (unsigned i = 0; i < sizeof(c_rowPlanCases) / sizeof(const char*); ++i)
        //     {
        //         VerifyRoundtripCase<RowPlan>(c_rowPlanCases[i]);
        //     }
        // }


        //*********************************************************************
        //
        // RowMatchNode::Builder cases.
        //
        //*********************************************************************
        void VerifyRowMatchNodeBuilderCase(RowMatchNode::Builder & builder,
                                           char const * expected)
        {
            RowMatchNode const * node = builder.Complete();
            if (node == nullptr)
            {
                EXPECT_EQ(expected[0], 0);
            }
            else
            {
                std::stringstream output;
                TextObjectFormatter formatter(output);
                node->Format(formatter);
                EXPECT_STREQ(expected, output.str().c_str());
            }
        }


        //*********************************************************************
        //
        // Build Row.
        //
        //*********************************************************************
        TEST(RowPlan,BuildRow)
        {
            Allocator allocator(128);
            RowMatchNode::Row row(AbstractRow(1, 1, false));
            RowMatchNode::Builder builder(row, allocator);
            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Row(1, 1, 0, false)");
        }


        //*********************************************************************
        //
        // Build And.
        //
        //*********************************************************************
        TEST(RowPlan,BuildAndEmpty)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::AndMatch, allocator);
            VerifyRowMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(RowPlan,BuildAndOneRow)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::AndMatch, allocator);

            RowMatchNode::Row row(AbstractRow(1, 1, false));
            builder.AddChild(&row);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Row(1, 1, 0, false)");
        }


        TEST(RowPlan,BuildAndTwoRows)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::AndMatch, allocator);

            RowMatchNode::Row row1(AbstractRow(1, 1, false));
            builder.AddChild(&row1);

            RowMatchNode::Row row2(AbstractRow(2, 2, false));
            builder.AddChild(&row2);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "And {\n"
                "  Children: [\n"
                "    Row(2, 2, 0, false),\n"
                "    Row(1, 1, 0, false)\n"
                "  ]\n"
                "}");
        }


        TEST(RowPlan,BuildAndThreeRows)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::AndMatch, allocator);

            RowMatchNode::Row row1(AbstractRow(1, 1, false));
            builder.AddChild(&row1);

            RowMatchNode::Row row2(AbstractRow(2, 2, false));
            builder.AddChild(&row2);

            RowMatchNode::Row row3(AbstractRow(3, 3, false));
            builder.AddChild(&row3);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "And {\n"
                "  Children: [\n"
                "    Row(3, 3, 0, false),\n"
                "    Row(2, 2, 0, false),\n"
                "    Row(1, 1, 0, false)\n"
                "  ]\n"
                "}");
        }


        //*********************************************************************
        //
        // Build Or.
        //
        //*********************************************************************
        TEST(RowPlan,BuildOrEmpty)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::OrMatch, allocator);
            VerifyRowMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(RowPlan,BuildOrOneRow)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::OrMatch, allocator);

            RowMatchNode::Row row(AbstractRow(1, 1, false));
            builder.AddChild(&row);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Row(1, 1, 0, false)");
        }


        TEST(RowPlan,BuildOrTwoRows)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::OrMatch, allocator);

            RowMatchNode::Row row1(AbstractRow(1, 1, false));
            builder.AddChild(&row1);

            RowMatchNode::Row row2(AbstractRow(2, 2, false));
            builder.AddChild(&row2);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Or {\n"
                "  Children: [\n"
                "    Row(2, 2, 0, false),\n"
                "    Row(1, 1, 0, false)\n"
                "  ]\n"
                "}");
        }


        TEST(RowPlan,BuildOrThreeRows)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::OrMatch, allocator);

            RowMatchNode::Row row1(AbstractRow(1, 1, false));
            builder.AddChild(&row1);

            RowMatchNode::Row row2(AbstractRow(2, 2, false));
            builder.AddChild(&row2);

            RowMatchNode::Row row3(AbstractRow(3, 3, false));
            builder.AddChild(&row3);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Or {\n"
                "  Children: [\n"
                "    Row(3, 3, 0, false),\n"
                "    Row(2, 2, 0, false),\n"
                "    Row(1, 1, 0, false)\n"
                "  ]\n"
                "}");
        }


        //*********************************************************************
        //
        // Build Not.
        //
        //*********************************************************************
        TEST(RowPlan,BuildNotEmpty)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::NotMatch, allocator);
            VerifyRowMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(RowPlan,BuildNotOneRow)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::NotMatch, allocator);

            RowMatchNode::Row row(AbstractRow(1, 1, false));
            builder.AddChild(&row);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Row(1, 1, 0, true)");
        }


        TEST(RowPlan,BuildNotNotOneRow)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::NotMatch, allocator);

            RowMatchNode::Row row(AbstractRow(1, 1, false));
            RowMatchNode::Not notNode(row);
            builder.AddChild(&notNode);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Row(1, 1, 0, false)");
        }


        TEST(RowPlan,BuildNotOneAnd)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::NotMatch, allocator);

            RowMatchNode::Row row1(AbstractRow(1, 0, false));
            RowMatchNode::Row row2(AbstractRow(2, 0, false));
            RowMatchNode::And andNode(row1, row2);
            builder.AddChild(&andNode);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "Not {\n"
                "  Child: And {\n"
                "    Children: [\n"
                "      Row(1, 0, 0, false),\n"
                "      Row(2, 0, 0, false)\n"
                "    ]\n"
                "  }\n"
                "}");
        }


        TEST(RowPlan,BuildNotNotOneAnd)
        {
            Allocator allocator(128);
            RowMatchNode::Builder builder(RowMatchNode::NotMatch, allocator);

            RowMatchNode::Row row1(AbstractRow(1, 0, false));
            RowMatchNode::Row row2(AbstractRow(2, 0, false));
            RowMatchNode::And andNode(row1, row2);
            RowMatchNode::Not notNode(andNode);
            builder.AddChild(&notNode);

            VerifyRowMatchNodeBuilderCase(
                 builder,
                "And {\n"
                "  Children: [\n"
                "    Row(1, 0, 0, false),\n"
                "    Row(2, 0, 0, false)\n"
                "  ]\n"
                "}");
        }
	}
}
