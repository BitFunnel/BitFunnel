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
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Utilities/TextObjectFormatter.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace TermPlanTest
    {
        const char* c_termMatchNodeCases[] = {
            //
            // TermMatchNodes
            //

            // Term nodes.
            "Unigram(\"123\", 0)",
            "Unigram(\"12f3\", 1)",
            "Unigram(\"12\\\"3\", 2)",
            "Unigram(\"12\\\\3\", 3)",


            // Term node with stream suffix.
            "Unigram(\"123\", 4)",


            // Fact node.
            "Fact(1)",


            // Empty And no longer allowed.
            // And with one child no longer allowed.


            // And node with children.
            "And {\n"
            "  Children: [\n"
            "    Unigram(\"123\", 0),\n"
            "    Unigram(\"12f3\", 1)\n"
            "  ]\n"
            "}",


            // Empty Or no longer allowed.
            // Or with one child no longer allowed.


            // Or node with children.
            "Or {\n"
            "  Children: [\n"
            "    Unigram(\"123\", 0),\n"
            "    Unigram(\"12f3\", 1)\n"
            "  ]\n"
            "}",


            // Not node.
            "Not {\n"
            "  Child: Unigram(\"123\", 0)\n"
            "}",


            // Phrase node with three terms with various arguments.
            "Phrase {\n"
            "  StreamId: 0,\n"
            "  Grams: [\n"
            "    \"123\",\n"
            "    \"456\",\n"
            "    \"789\"\n"
            "  ]\n"
            "}",


            // Complex tree where And and Or nodes each have multiple children.
            "And {\n"
            "  Children: [\n"
            "    Or {\n"
            "      Children: [\n"
            "        Phrase {\n"
            "          StreamId: 1,\n"
            "          Grams: [\n"
            "            \"123\",\n"
            "            \"456\",\n"
            "            \"789\"\n"
            "          ]\n"
            "        },\n"
            "        Unigram(\"123\", 0)\n"
            "      ]\n"
            "    },\n"
            "    Not {\n"
            "      Child: And {\n"
            "        Children: [\n"
            "          Unigram(\"123\", 0),\n"
            "          Unigram(\"foobar\", 1)\n"
            "        ]\n"
            "      }\n"
            "    }\n"
            "  ]\n"
            "}",
        };


        template <class T>
        void VerifyRoundtripCase(const char* text)
        {
            std::stringstream input(text);

            Allocator allocator(4096);
            TextObjectParser parser(input, allocator, &TermMatchNode::GetType);

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
        TEST(TermPlanTest, TermMatchNode)
        {
            for (unsigned i = 0; i < sizeof(c_termMatchNodeCases) / sizeof(const char*); ++i)
            {
                VerifyRoundtripCase<TermMatchNode>(c_termMatchNodeCases[i]);
            }
        }


        //*********************************************************************
        //
        // TermMatchNode::Builder cases.
        //
        //*********************************************************************
        void VerifyTermMatchNodeBuilderCase(TermMatchNode::Builder & builder,
                                            char const * expected)
        {
            TermMatchNode const * node = builder.Complete();
            if (node == nullptr)
            {
                ASSERT_EQ(expected[0], 0);
            }
            else
            {
                std::stringstream output;
                TextObjectFormatter formatter(output);
                node->Format(formatter);
                EXPECT_STREQ(expected, output.str().c_str());
            }
        }


        TermMatchNode const & BuildTermMatchTree(IAllocator& allocator,
                                                 char const * text)
        {
            std::stringstream input(text);
            TextObjectParser parser(input, allocator, &TermMatchNode::GetType);

            return TermMatchNode::Parse(parser);
        }


        void VerifyTermMatchLeafNode(IAllocator& allocator,
                                     char const * text)
        {
            TermMatchNode const & node = BuildTermMatchTree(allocator, text);
            TermMatchNode::Builder builder(node, allocator);
            VerifyTermMatchNodeBuilderCase(builder, text);
        }


        //*********************************************************************
        //
        // Build leaf nodes (Phrase, Unigram).
        //
        //*********************************************************************
        TEST(TermPlanTest, BuildPhraseMatch)
        {
            Allocator allocator(4096);
            VerifyTermMatchLeafNode(
                allocator,
                "Phrase {\n"
                "  StreamId: 0,\n"
                "  Grams: [\n"
                "    \"foo\",\n"
                "    \"bar\"\n"
                "  ]\n"
                "}");
        }


        TEST(TermPlanTest, BuildUnigramMatch)
        {
            Allocator allocator(4096);
            VerifyTermMatchLeafNode(
                allocator,
                "Unigram(\"foobar\", 0)");
        }


        //*********************************************************************
        //
        // Build And.
        //
        //*********************************************************************
        TEST(TermPlanTest, BuildAndEmpty)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);
            VerifyTermMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(TermPlanTest, BuildAndOneTerm)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term("foo", 0);
            builder.AddChild(&term);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Unigram(\"foo\", 0)");
        }


        TEST(TermPlanTest, BuildAndTwoRows)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term1("foo", 0);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("bar", 1);
            builder.AddChild(&term2);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "And {\n"
                "  Children: [\n"
                "    Unigram(\"bar\", 1),\n"
                "    Unigram(\"foo\", 0)\n"
                "  ]\n"
                "}");
        }


        TEST(TermPlanTest, BuildAndThreeRows)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term1("foo", 0);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("bar", 1);
            builder.AddChild(&term2);

            TermMatchNode::Unigram term3("baz", 2);
            builder.AddChild(&term3);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "And {\n"
                "  Children: [\n"
                "    Unigram(\"baz\", 2),\n"
                "    Unigram(\"bar\", 1),\n"
                "    Unigram(\"foo\", 0)\n"
                "  ]\n"
                "}");
        }


        //*********************************************************************
        //
        // Build Or.
        //
        //*********************************************************************
        TEST(TermPlanTest, BuildOrEmpty)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);
            VerifyTermMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(TermPlanTest, BuildOrOneTerm)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);

            TermMatchNode::Unigram term("foo", 0);
            builder.AddChild(&term);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Unigram(\"foo\", 0)");
        }


        TEST(TermPlanTest, BuildOrTwoRows)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);

            TermMatchNode::Unigram term1("foo", 0);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("\"bar\\", 1);
            builder.AddChild(&term2);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Or {\n"
                "  Children: [\n"
                "    Unigram(\"\\\"bar\\\\\", 1),\n"
                "    Unigram(\"foo\", 0)\n"
                "  ]\n"
                "}");
        }


        TEST(TermPlanTest, BuildOrThreeRows)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);

            TermMatchNode::Unigram term1("foo", 0);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("bar", 1);
            builder.AddChild(&term2);

            TermMatchNode::Unigram term3("baz", 2);
            builder.AddChild(&term3);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Or {\n"
                "  Children: [\n"
                "    Unigram(\"baz\", 2),\n"
                "    Unigram(\"bar\", 1),\n"
                "    Unigram(\"foo\", 0)\n"
                "  ]\n"
                "}");
        }


        //*********************************************************************
        //
        // Build Not.
        //
        //*********************************************************************
        TEST(TermPlanTest, BuildNotEmpty)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::NotMatch, allocator);
            VerifyTermMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(TermPlanTest, BuildNotOneRow)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::NotMatch, allocator);

            TermMatchNode::Unigram term1("\"", 0);
            builder.AddChild(&term1);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Not {\n"
                "  Child: Unigram(\"\\\"\", 0)\n"
                "}");
        }


        TEST(TermPlanTest, BuildNotNotOneRow)
        {
            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::NotMatch, allocator);

            TermMatchNode::Unigram term1("foo", 0);
            TermMatchNode::Not notNode(term1);
            builder.AddChild(&notNode);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Unigram(\"foo\", 0)");
        }


        TEST(TermPlanTest, BuildFactRowAndTerm)
        {
            static const FactHandle c_factHandle = 10;

            Allocator allocator(4096);
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term1("foo", 0);
            builder.AddChild(&term1);

            TermMatchNode::Fact fact(c_factHandle);
            builder.AddChild(&fact);

            VerifyTermMatchNodeBuilderCase(
                builder,
                "And {\n"
                "  Children: [\n"
                "    Fact(10),\n"
                "    Unigram(\"foo\", 0)\n"
                "  ]\n"
                "}");
        }
    }
}
