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

#pragma once

#include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.


namespace BitFunnel
{
    class IAllocator;


    //*************************************************************************
    //
    // MatchTreeRewriter rewrites a tree or RowMatchNode in a form that meets
    // all of the requirements of the RankDownCompiler.
    //
    // After rewrite, the tree will consist of an and-expression of
    //   1. a sequence of rows with non-zero rank in descending rank order
    //   2. followed by either
    //      a. a sequence of one or more or-expressions of rewritten
    //         RowMatchNode trees.
    //      b. an and expression of zero or more rank-0 rows followed by an
    //         optional RowMatchNode tree rooted by a Not node.
    //
    // The idea is to pull common, higher rank rows to the left, while
    // while distributing rank zero rows and complex not expressions over
    // any or-expressions.
    //
    //*************************************************************************
    class MatchTreeRewriter
    {
    public:
        // Rewrites the input tree.
        //
        // root:
        // Tree to be rewritten. Nodes in the return tree
        // are shared with the input tree or created in memory from the
        // allocator.
        //
        // targetRowCount:
        // Specifies how much of the tree to rewrite. Basically, the rewrite
        // will continue deeper into the tree until every path from the root
        // of the rewritten tree references at least the targetRowCount number
        // of distinct rows.
        //
        // targetCrossProductTermCount:
        // The rewriter attempts to form cross-products of OR-expressions
        // (e.g. replacing (a + b)(c + d) with ac + ad + bc + bd) in order to
        // generate longer sequences of row intersections which can be quickly
        // processed by the RankDown matching algorithm. Because the size of a
        // cross-product is exponential in the number of factors
        // (e.g. (a + b)(c + d)(e + f)(g + h) has 16 terms), it is often not
        // practical to generate complete cross-products. The targetCrossProductTermCount
        // specifies the maximum number of terms that can be generated before
        // halting subsequent cross-product expansions. Note that the system
        // may generate terms in excess of the target value. For example,
        // with a target of 3, the expression (a + b)(c + d)(e + f) would be
        // expanded to four terms, (ac + ad + bc + bd)(e + f), an amount
        // that is one greater than the target.
        static RowMatchNode const & Rewrite(RowMatchNode const & root,
                                            unsigned targetRowCount,
                                            unsigned targetCrossProductTermCount,
                                            IAllocator& allocator);

    private:
        // Partition is a helper class that divides the and-expression at the
        // root of a tree into individual rows or various ranks, or-expressions,
        // and not-expressions.
        class Partition : NonCopyable
        {
        public:
            Partition(IAllocator& allocator);
            Partition(Partition const & parent,
                      RowMatchNode const & node);

            bool HasOrTree() const;
            RowMatchNode::Or const & PopFromOrTree();

            unsigned GetRowCount() const;

            RowMatchNode const * RemoveRankNTree();

            RowMatchNode const & CreateTree() const;

            RowMatchNode::Or const & CreateOrNode(RowMatchNode const & left,
                                                  RowMatchNode const & right) const;

            RowMatchNode::And const & CreateAndNode(RowMatchNode const & left,
                                                    RowMatchNode const & right) const;

        private:
            void ProcessTree(RowMatchNode const & node);

            void AddNode(RowMatchNode const * & tree,
                         RowMatchNode const * node) const;

            void CreateReportNode(RowMatchNode const * & reportNode, RowMatchNode const * node) const;

            // Given an existing RowMatchTree rooted at RowMatchNode node, create a
            // new RowMatchTree. The new RowMatchTree is exactly the same as the existing
            // RowMatchTree with the exception that all non-rank0 rows in the existing
            // RowMatchTree are converted to a rank0 row by doing a RankUp operation on the
            // existing row.
            // The root node of the new RowMatchTree is returned. The notNodeEncountered boolean
            // variable indicates whether any NOT node is encountered during the rank up operation.
            // Based on the fact that whether any NOT node is inside the RowMatchTree rooted at node,
            // different compilers, which are rank down compiler or rank zero compiler, will be
            // chosen to compile the returned RowMatchTree.
            RowMatchNode const & RankUpToRankZero(RowMatchNode const & node, bool& notNodeEncountered) const;

            IAllocator& m_allocator;

            // Maintains the total number of rows on the path from the match
            // tree root through all parent partitions and all rows in the tio
            // level and-expression of this partition. Used to determine when
            // there are enough rows above to stop rewritting.
            unsigned m_rowCount;

            // During constructed from a parent Partition, m_parentRank is
            // initialized to the minimum rank value of the parent. Otherwise
            // m_parentRank is initialized to c_maxRankValue. m_parentRank is
            // used to determine if a row is being placed out of order. A row
            // is placed out of order when its rank is higher than the lowest
            // rank row processed so far. The RankDown matching algorithm is
            // most efficient when rows are processed from higher rank to
            // lower rank. For some expressions, it is not possible to arrange
            // all of the rows by descending rank. In this case, out of order
            // rows can be detected by comparing the row rank with
            // m_parentRank. When the row rank is greater than m_parentRank,
            // the row is marked as out of order so that it can be handled
            // specially by the RankDownCompiler.
            Rank m_parentRank;

            // Maintains the lowest non-zero rank encountered while
            // partitioning the top-level and expression. This value is used
            // as the parent rank for child partitions.
            Rank m_minRank;

            // Storage for and-expressions of rows encountered, organized by
            // row-rank. Entries are nullptr for ranks that have no rows. A non
            // nullptr entry consists of either a single row or an and-expression
            // of rows. The and-nodes are allocated from m_allocator.
            RowMatchNode const * m_rows[c_maxRankValue + 1];

            // The top of the tree is partitioned into an and expression of
            // four trees:
            //   m_rankNTree: and-expression of nodes with rank > 0, nodes
            //                ordered by decreasing rank.
            //   m_orTree: an or-node
            //   m_rank0Tree: an and-expression of rank-0 nodes.
            //   m_otherTree: an and-expression of not-nodes and match trees
            //                that were not rewritten because the target row
            //                count was met.
            // The and-expression of these four trees is equivalent to the
            // input tree.
            RowMatchNode const * m_rankNTree;
            RowMatchNode const * m_orTree;
            RowMatchNode const * m_rank0Tree;
            RowMatchNode const * m_otherTree;
        };

        // BuildCompileTree() builds a single tree from the partitioned tree
        // consisting of m_rankNTree, m_orTree, m_rank0Tree, and m_otherTree.
        // Note that BuildCompileTree() recursively builds the compile tree
        // for the left and right children of m_orTree.
        //
        // The rewriter attempts to form cross-products of OR-expressions
        // (e.g. replacing (a + b)(c + d) with ac + ad + bc + bd) in order
        // to generate longer sequences of row intersections which can be quickly
        // processed by the RankDown matching algorithm. Because the size of a
        // cross-product is exponential in the number of factors
        // (e.g. (a + b)(c + d)(e + f)(g + h) has 16 terms), it is often not
        // practical to generate complete cross-products. The targetCrossProductTermCount
        // specifies the maximum number of terms that can be generated before halting
        // subsequent cross-product expansions. Note that the system may generate
        // terms in excess of the target value. For example, with a target of 3,
        // the expression (a + b)(c + d)(e + f) would be expanded to four terms,
        // (ac + ad + bc + bd)(e + f), an amount that is one greater than the target.
        //
        // The currentCrossProductTermCount paramter accumulates the number of subtrees
        // generated while expanding cross-products of OR-expressions. The expansion
        // of cross-products terminates when the number of subtrees generated exceeds
        // the target amount.
        //
        // DESIGN NOTE: targetCrossProductTermCount is used as a "soft threshold" for the
        // number of terms can be generated while expanding cross-products. The actual
        // number of terms generated out can be slightly higher than this number depends
        // on the shape of the input tree.
        static RowMatchNode const & BuildCompileTree(Partition const & parent,
                                                     RowMatchNode const & node,
                                                     unsigned rowsRequired,
                                                     unsigned targetCrossProductTermCount,
                                                     unsigned& currentCrossProductTermCount);
    };
}
