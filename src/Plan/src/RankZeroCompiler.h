#pragma once

#include "BitFunnel/BitFunnelTypes.h"     // Rank used as a parameter.
#include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.


namespace BitFunnel
{
    class CompileNode;
    class IAllocator;
    class RowMatchNode;


    class RankZeroCompiler : NonCopyable
    {
    public:
        RankZeroCompiler(IAllocator& allocator);

        CompileNode const & Compile(RowMatchNode const & node);

    private:
        IAllocator& m_allocator;
    };
}
