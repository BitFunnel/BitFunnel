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

#include <sstream>

#include "AbstractRowEnumerator.h"
#include "BitFunnel/Allocators/IAllocator.h"
// #include "BitFunnel/BitFunnelErrors.h"
#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Utilities/TextObjectFormatter.h"
#include "BitFunnel/Utilities/RingBuffer.h"
#include "PlanRows.h"
#include "RowMatchNode.h"
#include "StringVector.h"
#include "TermMatchTreeConverter.h"



namespace BitFunnel
{
    TermMatchTreeConverter::TermMatchTreeConverter(const ISimpleIndex& index,
                                                   PlanRows& planRows,
                                                   // bool generateNonBodyPlan,
                                                   IAllocator& allocator)
        : m_allocator(allocator),
          m_index(index),
          m_planRows(planRows)
          // m_generateNonBodyPlan(generateNonBodyPlan),

    {
    }


    const RowMatchNode& TermMatchTreeConverter::BuildRowMatchTree(const TermMatchNode& root)
    {
         RowMatchNode::Builder builder(RowMatchNode::AndMatch, m_allocator);

         // It is important to build a node for the soft-deleted row first
         // because there are certain optimizations which cause some of the rows
         // to be ignored (e.g. due to number of rows exceeding the limit) and
         // the soft-deleted row must always be added.
         builder.AddChild(BuildDocumentActiveMatchNode());

         const RowMatchNode* mainMatchTree = BuildMatchTree(root);

         if (mainMatchTree == nullptr)
         {
             // TODO: handle error using BitFunnel error handling.
             std::stringstream output;
             output << "Conversion between TermPlan and RowPlan resulted in an empty MatchTree: \n";
             TextObjectFormatter formatter(output);
             root.Format(formatter);
             throw output.str().c_str();
         }
         else
         {
             builder.AddChild(mainMatchTree);

             const RowMatchNode* result = builder.Complete();

             return *result;
         }
    }


    const RowMatchNode* TermMatchTreeConverter::BuildDocumentActiveMatchNode()
    {
        const Term documentActiveDocumentTerm = ITermTable::GetDocumentActiveTerm();
        AbstractRowEnumerator rowEnumerator(documentActiveDocumentTerm, m_planRows);
        LogAssertB(rowEnumerator.MoveNext(), "couldn't find documentActive row.");

        const RowMatchNode* result = RowMatchNode::Builder::CreateRowNode(rowEnumerator.Current(), m_allocator);

        LogAssertB(!rowEnumerator.MoveNext(), "found more than one documentActive row.");

        return result;
    }


    const RowMatchNode* TermMatchTreeConverter::BuildMatchTree(const TermMatchNode& node)
    {
        switch (node.GetType())
        {
        case TermMatchNode::AndMatch:
            return BuildMatchTree(dynamic_cast<const TermMatchNode::And&>(node));
        case TermMatchNode::NotMatch:
            return BuildMatchTree(dynamic_cast<const TermMatchNode::Not&>(node));
        case TermMatchNode::OrMatch:
            return BuildMatchTree(dynamic_cast<const TermMatchNode::Or&>(node));
        case TermMatchNode::PhraseMatch:
            return BuildMatchTree(dynamic_cast<const TermMatchNode::Phrase&>(node));
        case TermMatchNode::UnigramMatch:
            return BuildMatchTree(dynamic_cast<const TermMatchNode::Unigram&>(node));
        case TermMatchNode::FactMatch:
            return BuildMatchTree(dynamic_cast<const TermMatchNode::Fact&>(node));
        default:
            LogAbortB("Invalid node type.");
            return nullptr; // C4715
        }
    }


    const RowMatchNode* TermMatchTreeConverter::BuildMatchTree(const TermMatchNode::And& node)
    {
        RowMatchNode::Builder builder(RowMatchNode::AndMatch, m_allocator);

        builder.AddChild(BuildMatchTree(node.GetLeft()));
        builder.AddChild(BuildMatchTree(node.GetRight()));

        return builder.Complete();
    }


    const RowMatchNode* TermMatchTreeConverter::BuildMatchTree(const TermMatchNode::Not& node)
    {
        RowMatchNode::Builder builder(RowMatchNode::NotMatch, m_allocator);

        builder.AddChild(BuildMatchTree(node.GetChild()));

        return builder.Complete();
    }


    const RowMatchNode* TermMatchTreeConverter::BuildMatchTree(const TermMatchNode::Or& node)
    {
        RowMatchNode::Builder builder(RowMatchNode::OrMatch, m_allocator);

        builder.AddChild(BuildMatchTree(node.GetLeft()));
        builder.AddChild(BuildMatchTree(node.GetRight()));

        return builder.Complete();
    }


    const RowMatchNode* TermMatchTreeConverter::BuildMatchTree(const TermMatchNode::Phrase& node)
    {
        RowMatchNode::Builder builder(RowMatchNode::AndMatch, m_allocator);
        RingBuffer<Term, Term::c_log2MaxGramSize + 1> termBuffer;

        StringVector const & stringVector = node.GetGrams();
        for (unsigned i = 0; i < stringVector.GetSize(); ++i)
        {
            *termBuffer.PushBack() = GetUnigramTerm(stringVector[i],
                                                    node.GetStreamId());

            if (termBuffer.GetCount() == Term::c_maxGramSize)
            {
                ProcessNGramBuffer(builder, termBuffer);
            }
        }

        while (!termBuffer.IsEmpty())
        {
            ProcessNGramBuffer(builder, termBuffer);
        }

        return builder.Complete();
    }


    const RowMatchNode* TermMatchTreeConverter::BuildMatchTree(const TermMatchNode::Unigram& node)
    {
        RowMatchNode::Builder builder(RowMatchNode::AndMatch, m_allocator);

        AppendTermRows(builder, GetUnigramTerm(node.GetText(), node.GetStreamId()));

        // if (m_generateNonBodyPlan && node.GetStreamId() == BitFunnel::Full)
        // {
        //     AppendTermRows(builder, GetUnigramTerm(node.GetText(),  BitFunnel::NonBody));
        // }

        return builder.Complete();
    }


    const RowMatchNode* TermMatchTreeConverter::BuildMatchTree(const TermMatchNode::Fact& node)
    {
        RowMatchNode::Builder builder(RowMatchNode::AndMatch, m_allocator);
        AppendTermRows(builder, node.GetFact());
        return builder.Complete();
    }


    // TODO: is this method needed at all? In the old codebase, there was a
    // giant switch based on Classification in order to determine the Tier. The
    // rewrite contains neither Tier nor Classification.
    const Term TermMatchTreeConverter::GetUnigramTerm(char const * text, Term::StreamId streamId) const
    {
        // TODO: get rid of GetUnigramTerm? It's pointless when used like this.
        return Term(text, streamId, m_index.GetConfiguration());
    }


    void TermMatchTreeConverter::ProcessNGramBuffer(RowMatchNode::Builder& builder,
                                                    RingBuffer<Term, Term::c_log2MaxGramSize + 1>& gramBuffer)
    {
        const size_t count = gramBuffer.GetCount();
        LogAssertB(count > 0, "must have non-empty gram.");

        Term term(gramBuffer[0]);
        AppendTermRows(builder, term);
        for (size_t n = 1; n < count; ++n)
        {
            term.AddTerm(gramBuffer[n], m_index.GetConfiguration());
            AppendTermRows(builder, term);
        }
        gramBuffer.PopFront();
    }


    void TermMatchTreeConverter::AppendTermRows(RowMatchNode::Builder& builder, const Term& term)
    {
        // The m_planRows.IsFull() check is an optimization to avoid unnecessary work for the rows
        // which will be ignored in case that the PlanRows is full inside AbstractRowEnumerator.
        // Theoretically, this check can be added at an even higher level in the code such as
        // BuildMatchTree(Unigram) or BuildMatchTree(Phrase). We add this optimization code here because
        // we think checking the PlanRow is full or not at this level is already good enough
        // from a performance point of view.
        if (!m_planRows.IsFull())
        {
            AbstractRowEnumerator rowEnumerator(term, m_planRows);
            while (rowEnumerator.MoveNext())
            {
                builder.AddChild(RowMatchNode::Builder::CreateRowNode(rowEnumerator.Current(), m_allocator));
            }
        }
    }

    void TermMatchTreeConverter::AppendTermRows(RowMatchNode::Builder& builder, const FactHandle& fact)
    {
        // The m_planRows.IsFull() check is an optimization to avoid unnecessary work for the rows
        // which will be ignored in case that the PlanRows is full inside AbstractRowEnumerator.
        // Theoretically, this check can be added at an even higher level in the code such as
        // BuildMatchTree(Unigram) or BuildMatchTree(Phrase). We add this optimization code here because
        // we think checking the PlanRow is full or not at this level is already good enough
        // from a performance point of view.
        // TODO TFS 16063. Consider returning true/false from these methods
        // when IPlanRows is full. Consider combining Term/Fact code paths together.
        if (!m_planRows.IsFull())
        {
            AbstractRowEnumerator rowEnumerator(fact, m_planRows);
            while (rowEnumerator.MoveNext())
            {
                builder.AddChild(RowMatchNode::Builder::CreateRowNode(rowEnumerator.Current(),
                                                                      m_allocator));
            }
        }
        else
        {
            // TODO: handle error.
            // No space in IPlanRows for processing a fact TermMatchNode in the tree.
            // LogB(Logging::Warning,
            //      "IndexServe",
            //      "Row count limit reached for fact with FactHandle value of %u "
            //      "when processing a TermMatchNode",
            //      fact);
        }
    }
}
