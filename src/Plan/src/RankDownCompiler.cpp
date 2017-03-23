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

#include <algorithm>    // For std::max.

#include "BitFunnel/Allocators/IAllocator.h"
#include "CompileNode.h"
#include "LoggerInterfaces/Logging.h"
#include "RankDownCompiler.h"
#include "RankZeroCompiler.h"
#include "RowMatchNode.h"


namespace BitFunnel
{
    RankDownCompiler::RankDownCompiler(IAllocator& allocator)
        : m_allocator(allocator),
          m_currentRank(0),
          m_maxRank(0),
          m_accumulator(nullptr)
    {
    }


    void RankDownCompiler::Compile(RowMatchNode const & root)
    {
        CompileInternal(root, true);
    }


    Rank RankDownCompiler::GetMaximumRank()
    {
        return m_maxRank;
    }


    void RankDownCompiler::CompileInternal(RowMatchNode const & root, bool leftMostChild)
    {
        // Initialize m_currentRank and m_accumulator here so that CompileInternal()
        // can be called multiple times.
        SetRank(0);
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
            SetRank(initialRank);
        }
        LogAssertB(m_accumulator != nullptr,
                   "nullptr m_accumulator.");

        return *m_accumulator;
    }


    void RankDownCompiler::CompileTraversal(RowMatchNode const & node, bool leftmostChild)
    {
        switch (node.GetType())
        {
        case RowMatchNode::AndMatch:
            {
                RowMatchNode::And const & andNode = dynamic_cast<RowMatchNode::And const &>(node);
                CompileTraversal(andNode.GetRight(), false);
                CompileTraversal(andNode.GetLeft(), leftmostChild);
            }
            break;
        case RowMatchNode::OrMatch:
            {
                RowMatchNode::Or const & orNode = dynamic_cast<RowMatchNode::Or const &>(node);

                RankDownCompiler left(m_allocator);
                left.CompileInternal(orNode.GetLeft(), leftmostChild);
                RankDownCompiler right(m_allocator);
                right.CompileInternal(orNode.GetRight(), leftmostChild);

                Rank rank = (std::max)(left.m_currentRank,
                                       right.m_currentRank);

                m_accumulator = new (m_allocator.Allocate(sizeof(CompileNode::Or)))
                                    CompileNode::Or(left.CreateTree(rank),
                                                    right.CreateTree(rank));

                SetRank(rank);
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
                    SetRank(row.GetRank());
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


    void RankDownCompiler::SetRank(Rank rank)
    {
        m_currentRank = rank;
        if (m_currentRank > m_maxRank)
        {
            m_maxRank = m_currentRank;
        }
    }
}
