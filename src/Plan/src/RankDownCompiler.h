#pragma once

#include "BitFunnel/BitFunnelTypes.h"     // Rank used as a parameter.
#include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.


namespace BitFunnel
{
    namespace Allocators
    {
        class IAllocator;
    }

    class CompileNode;
    class RowMatchNode;


    class RankDownCompiler : NonCopyable
    {
    public:
        RankDownCompiler(IAllocator& allocator);

        // DESIGN NOTE: Normally we would prefer a modeless class where all of
        // the work happened in the constructor and the CompileNode const & was
        // accessed via a const method. In this case we've broken out separate
        // Compile() and Create() tree methods because when compiling an Or
        // node, we must first compile the children and examine their ranks
        // before we can supply the initalRank parameter.

        void Compile(RowMatchNode const & root);

        CompileNode const & CreateTree(Rank initialRank);

    private:
        void CompileInternal(RowMatchNode const & root, bool leftMostChild);

        void CompileTraversal(RowMatchNode const & node,
                              bool leftMostChild);

        CompileNode const & RankUp(CompileNode const & node);

        IAllocator& m_allocator;
        Rank m_currentRank;
        CompileNode const * m_accumulator;
    };
}
