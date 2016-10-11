#pragma once

#include <vector>

#include "ByteCodeInterpreter.h"
#include "BitFunnel/Index/RowId.h"



namespace BitFunnel
{
    class TermMatchNode;


    class SimplePlanner
    {
    public:
        SimplePlanner(TermMatchNode const & tree, ISimpleIndex const & index);
    private:
        void Compile(size_t pos, Rank rank);
        void RankDown(size_t pos, Rank rank);
        void ExtractRowIds(TermMatchNode const & node);

        std::vector<RowId> m_rows;
        ISimpleIndex const & m_index;
        ByteCodeGenerator m_code;
    };

}
