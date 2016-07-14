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


#include <functional>
#include <stddef.h>
#include <vector>

#include "gtest/gtest.h"
#include "Mocks/ChunkEventTracer.h"


namespace BitFunnel
{
    namespace ChunkReaderUnitTest
    {
        // Captures string literals with embedded '\0' characters (avoiding the
        // default behavior of truncating the string at the '\0'), and converts
        // to a std::vector<char>.
        template <size_t LENGTH>
        std::vector<char> ToCharVector(char const (&input)[LENGTH])
        {
            return std::vector<char>(input, input + LENGTH - 1);
        }


        // Create a ChunkReader to parse the `chunk` data, and hook it up to an
        // ChunkEventCounter, so that we can count the events it emits as it
        // parses this data.
        static void RunEventTracerTest(
            std::vector<char> const & chunk,
            std::function<void(Mocks::ChunkEventTracer &)> const test)
        {
            Mocks::ChunkEventTracer readEventCounter(chunk);

            test(readEventCounter);
        }


        // Parse chunk with 1 document, 2 streams, and no terms in the first
        // stream.
        TEST(ChunkReaderUnitTest, ZeroTermStream)
        {
            std::vector<char> const zeroTermStream = ToCharVector(
                // First document.
                "Title\0\0"
                "Body\0Dogs\0\0"
                "\0"

                // End of corpus.
                "\0");

            RunEventTracerTest(
                zeroTermStream,
                [](Mocks::ChunkEventTracer & tracer)
                {
                    std::stringstream trace;
                    trace
                        << "OnFileEnter" << std::endl
                        << "OnDocumentEnter;DocId: '0'" << std::endl
                        << "OnStreamEnter;streamName: 'Title'" << std::endl
                        << "OnStreamExit" << std::endl
                        << "OnStreamEnter;streamName: 'Body'" << std::endl
                        << "OnTerm;term: 'Dogs'" << std::endl
                        << "OnStreamExit" << std::endl
                        << "OnDocumentExit" << std::endl
                        << "OnFileExit" << std::endl;

                    EXPECT_EQ(trace.str(), tracer.Trace());
                });
        }


        // Parse chunk with 1 document, 2 streams, and 0 terms.
        TEST(ChunkReaderUnitTest, ZeroTermDocument)
        {
            std::vector<char> const zeroTermDocument = ToCharVector(
                // First docuement.
                "Title\0\0"
                "Body\0\0"
                "\0"

                // End of corpus.
                "\0");

            RunEventTracerTest(
                zeroTermDocument,
                [](Mocks::ChunkEventTracer & tracer)
                {
                    std::stringstream trace;
                    trace
                        << "OnFileEnter" << std::endl
                        << "OnDocumentEnter;DocId: '0'" << std::endl
                        << "OnStreamEnter;streamName: 'Title'" << std::endl
                        << "OnStreamExit" << std::endl
                        << "OnStreamEnter;streamName: 'Body'" << std::endl
                        << "OnStreamExit" << std::endl
                        << "OnDocumentExit" << std::endl
                        << "OnFileExit" << std::endl;

                    EXPECT_EQ(trace.str(), tracer.Trace());
                });
        }


        // Parse chunk with 1 document, 2 streams, and a missing title on the
        // first stream. Notably this causes us to fail to parse any terms from
        // the document.
        TEST(ChunkReaderUnitTest, StreamWithEmptyTitle)
        {
            std::vector<char> const zeroTermDocument = ToCharVector(
                "\0Dogs\0\0"
                "Body\0Dogs\0\0"
                "\0"

                // End of corpus.
                "\0");

            // Parse chunk with 1 document and 0 terms.
            RunEventTracerTest(
                zeroTermDocument,
                [](Mocks::ChunkEventTracer & tracer)
                {
                    std::stringstream trace;
                    trace
                        // Missing title
                        << "OnFileEnter" << std::endl
                        << "OnFileExit" << std::endl;

                    EXPECT_EQ(trace.str(), tracer.Trace());
                });
        }


        // Parse a simple example chunk.
        TEST(ChunkReaderUnitTest, SimpleChunk)
        {
            std::vector<char> const chunk = ToCharVector(
                // First document
                "Title\0Dogs\0\0"
                "Body\0Dogs\0are\0man's\0best\0friend.\0\0"
                "\0"

                // Second document
                "Title\0Cat\0Facts\0\0"
                "Body\0The\0internet\0is\0made\0of\0cats.\0\0"
                "\0"

                // Third document
                "Title\0More\0Cat\0Facts\0\0"
                "Body\0The\0internet\0really\0is\0made\0of\0cats.\0\0"
                "\0"

                // End of corpus
                "\0");

            // Parse chunk with 1 document and 0 terms.
            RunEventTracerTest(chunk, [](Mocks::ChunkEventTracer & tracer)
            {
                std::stringstream trace;
                trace
                    << "OnFileEnter" << std::endl
                    // TODO: The DocId here is always 0. We should fix this.
                    << "OnDocumentEnter;DocId: '0'" << std::endl
                    << "OnStreamEnter;streamName: 'Title'" << std::endl
                    << "OnTerm;term: 'Dogs'" << std::endl
                    << "OnStreamExit" << std::endl
                    << "OnStreamEnter;streamName: 'Body'" << std::endl
                    << "OnTerm;term: 'Dogs'" << std::endl
                    << "OnTerm;term: 'are'" << std::endl
                    << "OnTerm;term: 'man's'" << std::endl
                    << "OnTerm;term: 'best'" << std::endl
                    << "OnTerm;term: 'friend.'" << std::endl
                    << "OnStreamExit" << std::endl
                    << "OnDocumentExit" << std::endl
                    << "OnDocumentEnter;DocId: '0'" << std::endl
                    << "OnStreamEnter;streamName: 'Title'" << std::endl
                    << "OnTerm;term: 'Cat'" << std::endl
                    << "OnTerm;term: 'Facts'" << std::endl
                    << "OnStreamExit" << std::endl
                    << "OnStreamEnter;streamName: 'Body'" << std::endl
                    << "OnTerm;term: 'The'" << std::endl
                    << "OnTerm;term: 'internet'" << std::endl
                    << "OnTerm;term: 'is'" << std::endl
                    << "OnTerm;term: 'made'" << std::endl
                    << "OnTerm;term: 'of'" << std::endl
                    << "OnTerm;term: 'cats.'" << std::endl
                    << "OnStreamExit" << std::endl
                    << "OnDocumentExit" << std::endl
                    << "OnDocumentEnter;DocId: '0'" << std::endl
                    << "OnStreamEnter;streamName: 'Title'" << std::endl
                    << "OnTerm;term: 'More'" << std::endl
                    << "OnTerm;term: 'Cat'" << std::endl
                    << "OnTerm;term: 'Facts'" << std::endl
                    << "OnStreamExit" << std::endl
                    << "OnStreamEnter;streamName: 'Body'" << std::endl
                    << "OnTerm;term: 'The'" << std::endl
                    << "OnTerm;term: 'internet'" << std::endl
                    << "OnTerm;term: 'really'" << std::endl
                    << "OnTerm;term: 'is'" << std::endl
                    << "OnTerm;term: 'made'" << std::endl
                    << "OnTerm;term: 'of'" << std::endl
                    << "OnTerm;term: 'cats.'" << std::endl
                    << "OnStreamExit" << std::endl
                    << "OnDocumentExit" << std::endl
                    << "OnFileExit" << std::endl;

                EXPECT_EQ(trace.str(), tracer.Trace());
            });
        }
    }
}
