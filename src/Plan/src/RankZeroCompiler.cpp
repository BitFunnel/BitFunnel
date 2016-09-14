#include "stdafx.h"

#include <new>
#include <stddef.h>

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "CompileNodes.h"
#include "LoggerInterfaces/Logging.h"
#include "RankZeroCompiler.h"
#include "BitFunnel/RowMatchNodes.h"


namespace BitFunnel
{
    RankZeroCompiler::RankZeroCompiler(Allocators::IAllocator& allocator)
        : m_allocator(allocator)
    {
    }


    CompileNode const & RankZeroCompiler::Compile(RowMatchNode const & node)
    {
        CompileNode const * result = nullptr;

        switch (node.GetType())
        {
        case RowMatchNode::AndMatch:
            result = new (m_allocator.Allocate(sizeof(CompileNode::AndTree)))
                         CompileNode::AndTree(Compile(dynamic_cast<RowMatchNode::And const &>(node).GetLeft()),
                                              Compile(dynamic_cast<RowMatchNode::And const &>(node).GetRight()));
            break;
        case RowMatchNode::NotMatch:
            result = new (m_allocator.Allocate(sizeof(CompileNode::Not)))
                         CompileNode::Not(Compile(dynamic_cast<RowMatchNode::Not const &>(node).GetChild()));
            break;
        case RowMatchNode::OrMatch:
            result = new (m_allocator.Allocate(sizeof(CompileNode::OrTree)))
                         CompileNode::OrTree(Compile(dynamic_cast<RowMatchNode::Or const &>(node).GetLeft()),
                                             Compile(dynamic_cast<RowMatchNode::Or const &>(node).GetRight()));
            break;
        case RowMatchNode::RowMatch:
            {
                AbstractRow const & row = dynamic_cast<RowMatchNode::Row const &>(node).GetRow();
                result = new (m_allocator.Allocate(sizeof(CompileNode::LoadRow)))
                             CompileNode::LoadRow(row);
            }
            break;
        default:
            LogAbortB("Unsupported node type.");
        }

        return *result;
    }
}
