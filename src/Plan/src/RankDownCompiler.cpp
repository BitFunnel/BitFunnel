#include "stdafx.h"

#include <algorithm>    // For std::max.
#include <new>
#include <stddef.h>

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "CompileNodes.h"
#include "BitFunnel/ICodeGenerator.h"
#include "LoggerInterfaces/Logging.h"
#include "RankDownCompiler.h"
#include "RankZeroCompiler.h"
#include "BitFunnel/RowMatchNodes.h"


namespace BitFunnel
{
    RankDownCompiler::RankDownCompiler(Allocators::IAllocator& allocator)
        : m_allocator(allocator),
          m_currentRank(0),
          m_accumulator(nullptr)
    {
    }


    void RankDownCompiler::Compile(RowMatchNode const & root)
    {
        CompileInternal(root, true);
    }


    void RankDownCompiler::CompileInternal(RowMatchNode const & root, bool leftMostChild)
    {
        // Initialize m_currentRank and m_accumulator here so that CompileInternal()
        // can be called multiple times.
        m_currentRank = 0;
        m_accumulator = nullptr;
        CompileTraversal(root, leftMostChild);
    }


    CompileNode const & RankDownCompiler::CreateTree(Rank initialRank)
    {
        if (initialRank > m_currentRank)
        {
            m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::RankDown)))
                                CompileNode::RankDown(initialRank - m_currentRank,
                                                      *m_accumulator);
            m_currentRank = initialRank;
        }
        LogAssertB(m_accumulator != nullptr);

        return *m_accumulator;
    }


    void RankDownCompiler::CompileTraversal(RowMatchNode const & node, bool leftmostChild)
    {
        switch (node.GetType())
        {
        case RowMatchNode::AndMatch:
            {
                RowMatchNode::And const & and = dynamic_cast<RowMatchNode::And const &>(node);
                CompileTraversal(and.GetRight(), false);
                CompileTraversal(and.GetLeft(), leftmostChild);
            }
            break;
        case RowMatchNode::OrMatch:
            {
                RowMatchNode::Or const & or = dynamic_cast<RowMatchNode::Or const &>(node);

                RankDownCompiler left(m_allocator);
                left.CompileInternal(or.GetLeft(), leftmostChild);
                RankDownCompiler right(m_allocator);
                right.CompileInternal(or.GetRight(), leftmostChild);

                Rank rank = (std::max)(left.m_currentRank,
                                       right.m_currentRank);

                m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::Or)))
                                    CompileNode::Or(left.CreateTree(rank),
                                                    right.CreateTree(rank));

                m_currentRank = rank;
            }
            break;
        case RowMatchNode::ReportMatch:
            {
                RowMatchNode::Report const & report = dynamic_cast<RowMatchNode::Report const &>(node);

                CompileNode const * child = nullptr;
                if (report.GetChild() != nullptr)
                {
                    RankZeroCompiler compiler(m_allocator);
                    child = &compiler.Compile(*report.GetChild());
                }

                m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::Report)))
                                    CompileNode::Report(child);
            }
            break;
        case RowMatchNode::RowMatch:
            {
                AbstractRow const & row = dynamic_cast<RowMatchNode::Row const &>(node).GetRow();

                if (m_accumulator == nullptr)
                {
                    m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::Report)))
                                        CompileNode::Report(nullptr);
                }

                if (row.GetRank() > m_currentRank)
                {
                    m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::RankDown)))
                                        CompileNode::RankDown(row.GetRank() - m_currentRank,
                                                              *m_accumulator);
                    m_currentRank = row.GetRank();
                }

                if (leftmostChild)
                {
                    m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::LoadRowJz)))
                                        CompileNode::LoadRowJz(row, *m_accumulator);
                }
                else
                {
                    m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::AndRowJz)))
                                        CompileNode::AndRowJz(row, *m_accumulator);
                }
            }
            break;
        default:
            LogAbortB("Bad RowMatchNode type.");
        };
    }
}
