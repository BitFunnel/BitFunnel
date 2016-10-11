#pragma once

#include <vector>

#include "ByteCodeInterpreter.h"
#include "BitFunnel/Index/RowId.h"
#include "BitFunnel/Plan/IResultsProcessor.h"



namespace BitFunnel
{
    class TermMatchNode;


    class SimplePlanner : public IResultsProcessor
    {
    public:
        SimplePlanner(TermMatchNode const & tree, ISimpleIndex const & index);
        virtual void AddResult(uint64_t accumulator,
                               size_t offset) override;

        virtual bool FinishIteration(void const * sliceBuffer) override;
        virtual bool TerminatedEarly() const override;
    private:
        void Compile(size_t pos, Rank rank);
        void RankDown(size_t pos, Rank rank);
        void ExtractRowIds(TermMatchNode const & node);

        std::vector<RowId> m_rows;
        ISimpleIndex const & m_index;
        ByteCodeGenerator m_code;

        // accumulator:offset pair.
        std::vector<std::pair<uint64_t, size_t>> m_addResultValues;
        std::vector<size_t> m_matches;
    };

}
