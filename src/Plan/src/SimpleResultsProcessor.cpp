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



#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Plan/Factories.h"
#include "SimpleResultsProcessor.h"

namespace BitFunnel
{
    std::unique_ptr<SimpleResultsProcessor> Factories::CreateSimpleResultsProcessor()
    {
        return std::unique_ptr<SimpleResultsProcessor>(new SimpleResultsProcessor);
    }


    std::vector<DocId> const & SimpleResultsProcessor::GetMatches() const
    {
        return m_matches;
    }


    void SimpleResultsProcessor::AddResult(uint64_t accumulator,
                                  size_t offset)
    {
        m_addResultValues.push_back(std::make_pair(accumulator, offset));
    }

    // TODO: move this method somewhere.
    uint64_t lzcnt(uint64_t value)
    {
#ifdef _MSC_VER
        return __lzcnt64(value);
#elif __LZCNT__
        return __lzcnt64(value);
#else
        return static_cast<uint64_t>(__builtin_clzll(value));
#endif
    }

    bool SimpleResultsProcessor::FinishIteration(void const * sliceBuffer)
    {
        for (auto const & result : m_addResultValues)
        {
            uint64_t acc = result.first;
            size_t offset = result.second;

            size_t bitPos = 63;

            while (acc != 0)
            {
                uint64_t count = lzcnt(acc);
                acc <<= count;
                bitPos -= count;

                DocIndex docIndex = offset * c_bitsPerQuadword + bitPos;
                DocumentHandle handle =
                    Factories::CreateDocumentHandle(const_cast<void*>(sliceBuffer), docIndex);
                m_matches.push_back(handle.GetDocId());

                acc <<= 1;
                --bitPos;
            }
        }
        m_addResultValues.clear();

        // TODO: don't always return false.
        return false;
    }


    bool SimpleResultsProcessor::TerminatedEarly() const
    {
        return false;
    }
}
