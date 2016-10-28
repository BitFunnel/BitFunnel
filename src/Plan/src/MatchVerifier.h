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
#include <vector>

#include "BitFunnel/Plan/IMatchVerifier.h"  // base class.

namespace BitFunnel
{
    class MatchVerifier : public IMatchVerifier
    {
    public:
        MatchVerifier(std::string query);

        virtual void AddExpected(DocId) override;
        virtual void AddObserved(DocId) override;

        virtual std::string GetQuery() const override;
        virtual std::vector<DocId> GetExpected() const override;
        virtual std::vector<DocId> GetObserved() const override;
        virtual std::vector<DocId> GetTruePositives() const override;
        virtual std::vector<DocId> GetFalsePositives() const override;
        virtual std::vector<DocId> GetFalseNegatives() const override;

        virtual size_t GetExpectedCount() const override;
        virtual size_t GetObservedCount() const override;
        virtual size_t GetTruePositiveCount() const override;
        virtual size_t GetFalsePositiveCount() const override;
        virtual size_t GetNumFalseNegatives() const override;

        virtual void Verify() override;
        virtual void Print(std::ostream &) const override;
        virtual void Reset() override;

    private:
        std::vector<DocId> m_expected;
        std::vector<DocId> m_observed;
        std::vector<DocId> m_truePositives;
        std::vector<DocId> m_falsePositives;
        std::vector<DocId> m_falseNegatives;

        std::string m_query;
    };
}
