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

#include "gtest/gtest.h"

#include "TermToText.h"


namespace BitFunnel
{
    namespace TermToTextTest
    {
        // Add then verify mappings added to a TermToText. 
        TEST(TermToText, AddTerm)
        {
            TermToText terms;

            Term::Hash maxHash = 100;
            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                std::string text = std::to_string(hash);
                terms.AddTerm(hash, text);
            }

            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                std::string expected = std::to_string(hash);
                std::string const & observed = terms.Lookup(hash);
                EXPECT_TRUE(expected.compare(observed) == 0);
            }
        }


        // Write TermToText to stream, then construct new TermToText from
        // from stream and verify contents.
        TEST(TermToText, RoundTrip)
        {
            TermToText terms;

            Term::Hash maxHash = 100;
            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                std::string text = std::to_string(hash);
                terms.AddTerm(hash, text);
            }

            std::stringstream stream;
            terms.Write(stream);
            TermToText terms2(stream);

            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                std::string expected = std::to_string(hash);
                std::string const & observed = terms2.Lookup(hash);
                EXPECT_TRUE(expected.compare(observed) == 0);
            }
        }
    }
}
