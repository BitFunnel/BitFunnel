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

#include <vector>       // std::vector parameter


namespace BitFunnel
{
    class ISimpleIndex;

    class QueryRunner
    {
    public:
        class Statistics
        {
        public:
            Statistics(size_t threadCount,
                       size_t uniqueQueryCount,
                       size_t processedCount,
                       double elapsedTime);

            void Print(std::ostream& out) const;

        private:
            const size_t m_threadCount;
            const size_t m_uniqueQueryCount;
            size_t m_processedCount;
            double m_elapsedTime;
        };

        static QueryInstrumentation::Data Run(
            char const * query,
            ISimpleIndex const & index,
            bool useNativeCode,
            bool countCacheLines);

        static Statistics Run(ISimpleIndex const & index,
                              char const * outputDir,
                              size_t threadCount,
                              std::vector<std::string> const & queries,
                              size_t iterations,
                              bool useNativeCode,
                              bool countCacheLines);
    };
}
