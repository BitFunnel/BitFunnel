#pragma once

#include <vector>

#include "BitFunnel/Plan/IResultsProcessor.h"

namespace BitFunnel
{
    class SimpleResultsProcessor : public IResultsProcessor
    {
    public:
        std::vector<DocId> const & GetMatches() const;

        //
        // IResultsProcessor
        //
        virtual void AddResult(uint64_t accumulator,
                               size_t offset) override;
        virtual bool FinishIteration(void const * sliceBuffer) override;
        virtual bool TerminatedEarly() const override;

    private:
        // accumulator:offset pair.
        std::vector<std::pair<uint64_t, size_t>> m_addResultValues;
        std::vector<size_t> m_matches;
    };
}
