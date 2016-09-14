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


    class RankZeroCompiler : NonCopyable
    {
    public:
        RankZeroCompiler(Allocators::IAllocator& allocator);

        CompileNode const & Compile(RowMatchNode const & node);

    private:
        Allocators::IAllocator& m_allocator;
    };
}
