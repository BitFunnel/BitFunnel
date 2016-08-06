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

#include <iostream>         // TODO: Remove this temporary include.
#include <sstream>

#include "gtest/gtest.h"

#include "DocumentFrequencyTable.h"
#include "TermTableBuilder.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    namespace TermTableBuilderTest
    {
        TEST(TermTableBuilder, InProgress)
        {
            std::stringstream input;
            input << "123,1,1,0.5\n456,1,1,0.1\n789,1,1,0.01\n";
            DocumentFrequencyTable terms(input);

            TreatmentPrivateRank0 treatment;

            double density = 0.1;
            double adhocFrequency = 0.01;

            TermTableBuilder builder(density, adhocFrequency, treatment, terms);
            builder.Print(std::cout);
        }
    }
}
