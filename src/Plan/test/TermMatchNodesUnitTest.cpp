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

#include "BitFunnel/TermMatchNodes.h"
#include "PrivateHeapAllocator.h"
#include "TextObjectFormatter.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace TermPlanUnitTest
    {
        const char* c_termMatchNodeCases[] = {
            //
            // TermMatchNodes
            //

            // Term nodes.
            "Unigram(\"123\", full)",
            "Unigram(\"12f3\", nonbody)",
            "Unigram(\"12\\\"3\", metaword)",
            "Unigram(\"12\\\\3\", clickboost)",


            // Term node with stream suffix.
            "Unigram(\"123\", full)",


            // Fact node.
            "Fact(1)",


            // Empty And no longer allowed.
            // And with one child no longer allowed.


            // And node with children.
            "And {\n"
            "  Children: [\n"
            "    Unigram(\"123\", full),\n"
            "    Unigram(\"12f3\", nonbody)\n"
            "  ]\n"
            "}",


            // Empty Or no longer allowed.
            // Or with one child no longer allowed.


            // Or node with children.
            "Or {\n"
            "  Children: [\n"
            "    Unigram(\"123\", full),\n"
            "    Unigram(\"12f3\", nonbody)\n"
            "  ]\n"
            "}",


            // Not node.
            "Not {\n"
            "  Child: Unigram(\"123\", full)\n"
            "}",


            // Phrase node with three terms with various arguments.
            "Phrase {\n"
            "  Classification: nonbody,\n"
            "  Suffix: nullable(),\n"
            "  Grams: [\n"
            "    \"123\",\n"
            "    \"456\",\n"
            "    \"789\"\n"
            "  ]\n"
            "}",


            // Phrase node with three terms and stream suffix.
            "Phrase {\n"
            "  Classification: nonbody,\n"
            "  Suffix: nullable(\"stream\"),\n"
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
            "          Classification: nonbody,\n"
            "          Suffix: nullable(),\n"
            "          Grams: [\n"
            "            \"123\",\n"
            "            \"456\",\n"
            "            \"789\"\n"
            "          ]\n"
            "        },\n"
            "        Unigram(\"123\", full)\n"
            "      ]\n"
            "    },\n"
            "    Not {\n"
            "      Child: And {\n"
            "        Children: [\n"
            "          Unigram(\"123\", full),\n"
            "          Unigram(\"foobar\", clickboost)\n"
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

            PrivateHeapAllocator allocator;
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
        TEST(TermPlanUnitTest, TermMatchNode)
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
        TEST(TermPlanUnitTest, BuildPhraseMatch)
        {
            PrivateHeapAllocator allocator;
            VerifyTermMatchLeafNode(
                allocator,
                "Phrase {\n"
                "  Classification: full,\n"
                "  Suffix: nullable(),\n"
                "  Grams: [\n"
                "    \"foo\",\n"
                "    \"bar\"\n"
                "  ]\n"
                "}");
        }


        TEST(TermPlanUnitTest, BuildUnigramMatch)
        {
            PrivateHeapAllocator allocator;
            VerifyTermMatchLeafNode(
                allocator,
                "Unigram(\"foobar\", metaword)");
        }


        //*********************************************************************
        //
        // Build And.
        //
        //*********************************************************************
        TEST(TermPlanUnitTest, BuildAndEmpty)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);
            VerifyTermMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(TermPlanUnitTest, BuildAndOneTerm)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term("foo", Full);
            builder.AddChild(&term);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Unigram(\"foo\", full)");
        }


        TEST(TermPlanUnitTest, BuildAndTwoRows)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term1("foo", Full);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("bar", MetaWord);
            builder.AddChild(&term2);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "And {\n"
                "  Children: [\n"
                "    Unigram(\"bar\", metaword),\n"
                "    Unigram(\"foo\", full)\n"
                "  ]\n"
                "}");
        }


        TEST(TermPlanUnitTest, BuildAndThreeRows)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term1("foo", Full);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("bar", MetaWord);
            builder.AddChild(&term2);

            TermMatchNode::Unigram term3("baz", ClickBoost);
            builder.AddChild(&term3);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "And {\n"
                "  Children: [\n"
                "    Unigram(\"baz\", clickboost),\n"
                "    Unigram(\"bar\", metaword),\n"
                "    Unigram(\"foo\", full)\n"
                "  ]\n"
                "}");
        }


        //*********************************************************************
        //
        // Build Or.
        //
        //*********************************************************************
        TEST(TermPlanUnitTest, BuildOrEmpty)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);
            VerifyTermMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(TermPlanUnitTest, BuildOrOneTerm)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);

            TermMatchNode::Unigram term("foo", Full);
            builder.AddChild(&term);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Unigram(\"foo\", full)");
        }


        TEST(TermPlanUnitTest, BuildOrTwoRows)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);

            TermMatchNode::Unigram term1("foo", Full);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("\"bar\\", NonBody);
            builder.AddChild(&term2);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Or {\n"
                "  Children: [\n"
                "    Unigram(\"\\\"bar\\\\\", nonbody),\n"
                "    Unigram(\"foo\", full)\n"
                "  ]\n"
                "}");
        }


        TEST(TermPlanUnitTest, BuildOrThreeRows)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::OrMatch, allocator);

            TermMatchNode::Unigram term1("foo", Full);
            builder.AddChild(&term1);

            TermMatchNode::Unigram term2("bar", MetaWord);
            builder.AddChild(&term2);

            TermMatchNode::Unigram term3("baz", NonBody);
            builder.AddChild(&term3);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Or {\n"
                "  Children: [\n"
                "    Unigram(\"baz\", nonbody),\n"
                "    Unigram(\"bar\", metaword),\n"
                "    Unigram(\"foo\", full)\n"
                "  ]\n"
                "}");
        }


        //*********************************************************************
        //
        // Build Not.
        //
        //*********************************************************************
        TEST(TermPlanUnitTest, BuildNotEmpty)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::NotMatch, allocator);
            VerifyTermMatchNodeBuilderCase(
                 builder,
                "");
        }


        TEST(TermPlanUnitTest, BuildNotOneRow)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::NotMatch, allocator);

            TermMatchNode::Unigram term1("\"", Full);
            builder.AddChild(&term1);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Not {\n"
                "  Child: Unigram(\"\\\"\", full)\n"
                "}");
        }


        TEST(TermPlanUnitTest, BuildNotNotOneRow)
        {
            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::NotMatch, allocator);

            TermMatchNode::Unigram term1("foo", Full);
            TermMatchNode::Not notNode(term1);
            builder.AddChild(&notNode);

            VerifyTermMatchNodeBuilderCase(
                 builder,
                "Unigram(\"foo\", full)");
        }


        TEST(TermPlanUnitTest, BuildFactRowAndTerm)
        {
            static const FactHandle c_factHandle = 10;

            PrivateHeapAllocator allocator;
            TermMatchNode::Builder builder(TermMatchNode::AndMatch, allocator);

            TermMatchNode::Unigram term1("foo", Full);
            builder.AddChild(&term1);

            TermMatchNode::Fact fact(c_factHandle);
            builder.AddChild(&fact);

            VerifyTermMatchNodeBuilderCase(
                builder,
                "And {\n"
                "  Children: [\n"
                "    Fact(10),\n"
                "    Unigram(\"foo\", full)\n"
                "  ]\n"
                "}");
        }
    }
}
