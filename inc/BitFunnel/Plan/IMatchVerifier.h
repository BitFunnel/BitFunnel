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

#pragma once

#include <iosfwd>
#include <string>

#include "BitFunnel/BitFunnelTypes.h"  // DocId parameter.
#include "BitFunnel/IInterface.h"      // IInterface base class.

namespace BitFunnel
{
    class IMatchVerifier : public IInterface
    {
    public:
        virtual void AddExpected(DocId id) = 0;
        virtual void AddObserved(DocId id) = 0;

        virtual std::string GetQuery() const = 0;
        virtual std::vector<DocId> GetExpected() const = 0;
        virtual std::vector<DocId> GetObserved() const = 0;
        virtual std::vector<DocId> GetTruePositives() const = 0;
        virtual std::vector<DocId> GetFalsePositives() const = 0;
        virtual std::vector<DocId> GetFalseNegatives() const = 0;

        virtual size_t GetExpectedCount() const = 0;
        virtual size_t GetObservedCount() const = 0;
        virtual size_t GetTruePositiveCount() const = 0;
        virtual size_t GetFalsePositiveCount() const = 0;
        virtual size_t GetFalseNegativeCount() const = 0;

        virtual void Verify() = 0;
        virtual void Print(std::ostream & out) const = 0;
        virtual void Reset() = 0;
    };
}
