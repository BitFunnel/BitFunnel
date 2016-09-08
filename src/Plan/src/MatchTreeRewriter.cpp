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

#include <new>          // For placement new.
#include <stddef.h>     // For nullptr.

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/RowMatchNode.h"
#include "LoggerInterfaces/Logging.h"
#include "MatchTreeRewriter.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // MatchTreeRewriter
    //
    //*************************************************************************
    RowMatchNode const & MatchTreeRewriter::Rewrite(RowMatchNode const & root,
                                                    unsigned targetRowCount,
                                                    unsigned targetCrossProductTermCount,
                                                    IAllocator& allocator)
    {
        Partition partition(allocator);

        unsigned currentCrossProductTermCount = 0;
        return BuildCompileTree(partition,
                                root,
                                targetRowCount,
                                targetCrossProductTermCount,
                                currentCrossProductTermCount);
    }


    RowMatchNode const & MatchTreeRewriter::BuildCompileTree(Partition const & parent,
                                                             RowMatchNode const & node,
                                                             unsigned targetRowCount,
                                                             unsigned targetCrossProductTermCount,
                                                             unsigned& currentCrossProductTermCount)
    {
        Partition partition(parent, node);

        // The rewriting recursion halts and the partition is converted directly to a tree
        // when any of the following three conditions are true:
        // 1. The partition has no more OR-trees with which to form cross products.
        // 2. The number of rows in the partition meets or exceeds targetRowCount. The goal
        //    is to process at least this many rows with the fast  RankDown matching algorithm.
        //    After the bitwise-AND of targetRowCount rows, matches are far enough apart to use
        //    the slower Rank0 matching algorithm.
        // 3. The number of cross product terms generated so far meets or exceeds the target cross
        //    product term count. Enforcing a limit on the number of cross product terms generated
        //    is essential because the size of a complete cross product is exponential in the
        //    number of factors.
        if (!partition.HasOrTree()
            || targetRowCount < partition.GetRowCount()
            || currentCrossProductTermCount >= targetCrossProductTermCount)
        {
            // The tree created in this block counts as one of the cross product terms.
            // Therefore increment the cross product term count.
            currentCrossProductTermCount++;
            return partition.CreateTree();
        }
        else
        {
            RowMatchNode const * rankNTree = partition.RemoveRankNTree();
            RowMatchNode::Or const & orNode(partition.PopFromOrTree());

            // Multiply out the left node of the OR tree to the partition.
            RowMatchNode const & left = BuildCompileTree(partition,
                                                         orNode.GetLeft(),
                                                         targetRowCount,
                                                         targetCrossProductTermCount,
                                                         currentCrossProductTermCount);

            // Multiply out the right node of the OR tree to the partition.
            RowMatchNode const & right = BuildCompileTree(partition,
                                                          orNode.GetRight(),
                                                          targetRowCount,
                                                          targetCrossProductTermCount,
                                                          currentCrossProductTermCount);
            RowMatchNode const & compiledOrTree = partition.CreateOrNode(left, right);

            if (rankNTree != nullptr)
            {
                return partition.CreateAndNode(*rankNTree, compiledOrTree);
            }
            else
            {
                return compiledOrTree;
            }
        }
    }


    //*************************************************************************
    //
    // MatchTreeRewriter::Partition
    //
    //*************************************************************************
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4351)
#endif
    MatchTreeRewriter::Partition::Partition(IAllocator& allocator)
        : m_allocator(allocator),
          m_rowCount(0),
          m_parentRank(c_maxRankValue),
          m_minRank(c_maxRankValue),
          m_rows(),
          m_rankNTree(nullptr),
          m_orTree(nullptr),
          m_rank0Tree(nullptr),
          m_otherTree(nullptr)
    {
    }
#ifdef _MSC_VER
#pragma warning(pop)


#pragma warning(push)
#pragma warning(disable:4351)
#endif
    MatchTreeRewriter::Partition::Partition(Partition const & parent,
                                            RowMatchNode const & node)
        : m_allocator(parent.m_allocator),
          m_rowCount(parent.m_rowCount),
          m_parentRank(parent.m_minRank),
          m_minRank(parent.m_minRank),
          m_rows(),
          m_rankNTree(parent.m_rankNTree),
          m_orTree(parent.m_orTree),
          m_rank0Tree(parent.m_rank0Tree),
          m_otherTree(parent.m_otherTree)
    {
        ProcessTree(node);

        AddNode(m_rank0Tree, m_rows[0]);

        for (Rank rank = 1; rank <= c_maxRankValue; ++rank)
        {
            AddNode(m_rankNTree, m_rows[rank]);
        }
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    bool MatchTreeRewriter::Partition::HasOrTree() const
    {
        return m_orTree != nullptr;
    }


    RowMatchNode::Or const & MatchTreeRewriter::Partition::PopFromOrTree()
    {
        LogAssertB(HasOrTree(), "Doesn't have OrTree");
        if (m_orTree->GetType() == RowMatchNode::AndMatch)
        {
            RowMatchNode::And const & andNode = *dynamic_cast<RowMatchNode::And const *>(m_orTree);
            m_orTree = &andNode.GetRight();
            return dynamic_cast<RowMatchNode::Or const &>(andNode.GetLeft());
        }
        else
        {
            RowMatchNode::Or const & result = dynamic_cast<RowMatchNode::Or const &>(*m_orTree);
            m_orTree = nullptr;
            return result;
        }
    }


    unsigned MatchTreeRewriter::Partition::GetRowCount() const
    {
        return m_rowCount;
    }


    RowMatchNode const * MatchTreeRewriter::Partition::RemoveRankNTree()
    {
        RowMatchNode const * result = m_rankNTree;
        m_rankNTree = nullptr;
        return result;
    }


    RowMatchNode const & MatchTreeRewriter::Partition::CreateTree() const
    {
        RowMatchNode const * tree = nullptr;

        if (m_orTree != nullptr)
        {
            // If the m_orTree is not nullptr, it means that the rewrite early terminated either
            // because the target row count limit was reached or the cross-product term count limit
            // was reached. In this case, since the or tree may have any possible combination of
            // rows in different ranks, we need to adjust all the rows under the Or-tree to be
            // rank zero rows so that it can be compiled using the RankDown compiler.
            bool containsNotNode = false;
            RowMatchNode const * orTreeAfterRankUp = &(RankUpToRankZero(*m_orTree, containsNotNode));

            if (containsNotNode)
            {
                // If there is at least one Not node inside the or tree, the orTreeAfterRoundup must be compiled
                // using rank zero compiler.
                // In this case, the report node returns the And of "for m_otherTree and orTreeAfterRoundup.
                AddNode(tree, m_otherTree);
                AddNode(tree, orTreeAfterRankUp);
                CreateReportNode(tree, tree);
            }
            else
            {
                // There are no Not nodes inside the or tree, so only need to put the other tree under a
                // report node (using rank zero compiler). The orTreeAfterRoundup can still be run with
                // the RankDown compiler.
                CreateReportNode(tree, m_otherTree);
                AddNode(tree, orTreeAfterRankUp);
            }
        }
        else
        {
            tree =  new (m_allocator.Allocate(sizeof(RowMatchNode::Report))) RowMatchNode::Report(m_otherTree);
        }

        AddNode(tree, m_rank0Tree);
        AddNode(tree, m_rankNTree);

        LogAssertB(tree != nullptr, "Null pointer");
        return *tree;
    }


    RowMatchNode::Or const &
        MatchTreeRewriter::Partition::CreateOrNode(RowMatchNode const & left,
                                                   RowMatchNode const & right) const
    {
        return *new (m_allocator.Allocate(sizeof(RowMatchNode::Or)))
                    RowMatchNode::Or(left, right);
    }


    RowMatchNode::And const &
        MatchTreeRewriter::Partition::CreateAndNode(RowMatchNode const & left,
                                                    RowMatchNode const & right) const
    {
        return *new (m_allocator.Allocate(sizeof(RowMatchNode::And)))
                    RowMatchNode::And(left, right);
    }


    void MatchTreeRewriter::Partition::ProcessTree(RowMatchNode const & node)
    {
        switch (node.GetType())
        {
        case RowMatchNode::AndMatch:
            ProcessTree(dynamic_cast<RowMatchNode::And const &>(node).GetLeft());
            ProcessTree(dynamic_cast<RowMatchNode::And const &>(node).GetRight());
            break;
        case RowMatchNode::NotMatch:
            {
                // For NOT node, adjust all the rows under the NOT node to be a rank
                // zero row. Then add the NOT node to the m_otherTree.
                // In this case, the value of the containsNotNode doesn't need to be
                // inspected.
                bool containsNotNode = false;
                RowMatchNode const & otherTreeAfterRankUp = RankUpToRankZero(node, containsNotNode);
                AddNode(m_otherTree, &otherTreeAfterRankUp);
                break;
            }
        case RowMatchNode::OrMatch:
            AddNode(m_orTree, &node);
            break;
        case RowMatchNode::RowMatch:
            {
                ++m_rowCount;

                AbstractRow row = dynamic_cast<RowMatchNode::Row const &>(node).GetRow();
                Rank rank = row.GetRank();

                if (rank > 0 && rank < m_minRank)
                {
                    m_minRank = rank;
                }

                if (rank > m_parentRank)
                {
                    RowMatchNode::Row* rankUpRow =
                        new (m_allocator.Allocate(sizeof(RowMatchNode::Row)))
                            RowMatchNode::Row(AbstractRow(row, rank - m_parentRank));
                    AddNode(m_rows[rank], rankUpRow);
                }
                else
                {
                    AddNode(m_rows[rank], &node);
                }
            }
            break;
        default:
            LogAbortB("Unsupported node type.");
        }
    }


    void MatchTreeRewriter::Partition::AddNode(RowMatchNode const * & tree, RowMatchNode const * node) const
    {
        if (node != nullptr)
        {
            if (tree == nullptr)
            {
                tree = node;
            }
            else
            {
                tree = new (m_allocator.Allocate(sizeof(RowMatchNode::And)))
                            RowMatchNode::And(*node, *tree);
            }
        }
    }


    void MatchTreeRewriter::Partition::CreateReportNode(RowMatchNode const * & reportNode,
                                                        RowMatchNode const * node) const
    {
        reportNode =  new (m_allocator.Allocate(sizeof(RowMatchNode::Report)))
                           RowMatchNode::Report(node);
    }


    RowMatchNode const & MatchTreeRewriter::Partition::RankUpToRankZero(RowMatchNode const & node, bool& containsNotNode) const
    {
        switch (node.GetType())
        {
        case RowMatchNode::AndMatch:
            {
                RowMatchNode::And const & andNode = dynamic_cast<RowMatchNode::And const &>(node);
                RowMatchNode const & leftNode = RankUpToRankZero(andNode.GetLeft(), containsNotNode);
                RowMatchNode const & rightNode = RankUpToRankZero(andNode.GetRight(), containsNotNode);
                return CreateAndNode(leftNode, rightNode);
            }
        case RowMatchNode::NotMatch:
            {
                RowMatchNode::Not const & notNode = dynamic_cast<RowMatchNode::Not const &>(node);
                RowMatchNode const & notNodeAfterRankup = RankUpToRankZero(notNode.GetChild(), containsNotNode);
                containsNotNode = true;
                return *new (m_allocator.Allocate(sizeof(RowMatchNode::Not)))
                            RowMatchNode::Not(notNodeAfterRankup);
            }
        case RowMatchNode::OrMatch:
            {
                RowMatchNode::Or const & orNode = dynamic_cast<RowMatchNode::Or const &>(node);
                RowMatchNode const & leftNode = RankUpToRankZero(orNode.GetLeft(), containsNotNode);
                RowMatchNode const & rightNode = RankUpToRankZero(orNode.GetRight(), containsNotNode);
                return CreateOrNode(leftNode, rightNode);
            }
        case RowMatchNode::RowMatch:
            {
                const AbstractRow row = dynamic_cast<RowMatchNode::Row const &>(node).GetRow();
                const Rank rank = row.GetRank();

                if (rank > 0)
                {
                    return *new (m_allocator.Allocate(sizeof(RowMatchNode::Row)))
                                RowMatchNode::Row(AbstractRow(row, rank));
                }
                else
                {
                    return node;
                }
            }
        default:
            LogAbortB("Unsupported node type.");
            return *(static_cast<const RowMatchNode::Row*>(nullptr));
        }
    }
}
