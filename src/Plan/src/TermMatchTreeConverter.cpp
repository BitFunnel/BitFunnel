#include <sstream>

#include "AbstractRowEnumerator.h"
#include "BitFunnel/Allocators/IAllocator.h"
// #include "BitFunnel/BitFunnelErrors.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/IIndexedIdfTable.h"
#include "BitFunnel/Plan/IPlanRows.h"
#include "LoggerInterfaces/Logging.h"
#include "PlanRows.h"
#include "RingBuffer.h"
#include "BitFunnel/RowMatchNode.h"
#include "BitFunnel/StringVector.h"
#include "TermMatchTreeConverter.h"
#include "TextObjectFormatter.h"


namespace BitFunnel
{
    TermMatchTreeConverter::TermMatchTreeConverter(const IIndexedIdfTable& idfTable,
                                                   PlanRows& planRows,
                                                   bool generateNonBodyPlan,
                                                   Allocators::IAllocator& allocator)
        : m_idfTable(idfTable),
          m_planRows(planRows),
          // m_generateNonBodyPlan(generateNonBodyPlan),
          m_allocator(allocator)
    {
    }


    const RowMatchNode& TermMatchTreeConverter::BuildRowMatchTree(const TermMatchNode& root)
    {
         RowMatchNode::Builder builder(RowMatchNode::AndMatch, m_allocator);

         // It is important to build a node for the soft-deleted row first
         // because there are certain optimizations which cause some of the rows
         // to be ignored (e.g. due to number of rows exceeding the limit) and
         // the soft-deleted row must always be added.
         builder.AddChild(BuildSoftDeletedMatchNode());

         const RowMatchNode* mainMatchTree = BuildMatchTree(root);

         if (mainMatchTree == nullptr)
         {
             std::stringstream output;
             output << "Conversion between TermPlan and RowPlan resulted in an empty MatchTree: \n";
             TextObjectFormatter formatter(output);
             root.Format(formatter);
             throw QueryError(output.str().c_str());
         }
         else
         {
             builder.AddChild(mainMatchTree);

             const RowMatchNode* result = builder.Complete();

             return *result;
         }
    }


    const RowMatchNode* TermMatchTreeConverter::BuildSoftDeletedMatchNode()
    {
        const Term softDeletedDocumentTerm = ITermTable::GetSoftDeletedTerm();
        AbstractRowEnumerator rowEnumerator(softDeletedDocumentTerm, m_planRows);
        LogAssertB(rowEnumerator.MoveNext());

        const RowMatchNode* result = RowMatchNode::Builder::CreateRowNode(rowEnumerator.Current(), m_allocator);

        // Soft deleted document should have exactly one row.
        LogAssertB(!rowEnumerator.MoveNext());

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
        RingBuffer<Term, c_log2MaxGramSize + 1> termBuffer;

        StringVector const & stringVector = node.GetGrams();
        for (unsigned i = 0; i < stringVector.GetSize(); ++i)
        {
            *termBuffer.PushBack() = GetUnigramTerm(stringVector[i], node.GetSuffix(), node.GetClassification());

            if (termBuffer.GetCount() == c_maxGramSize)
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

        AppendTermRows(builder, GetUnigramTerm(node.GetText(), node.GetSuffix(), node.GetClassification()));

        // if (m_generateNonBodyPlan && node.GetClassification() == BitFunnel::Full)
        // {
        //     AppendTermRows(builder, GetUnigramTerm(node.GetText(), node.GetSuffix(), BitFunnel::NonBody));
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
    const Term TermMatchTreeConverter::GetUnigramTerm(char const * text, char const * suffix, Classification classification) const
    {
        Term::Hash termHash = Term::ComputeQualifiedRawHash(text, suffix);
        IdfX10 termIdf = m_index.GetIndexedDocFreqTable().LookupIdf(termHash);

        const Stream::Classification termClassification =
            ClassificationToStreamClassification(classification);

        Tier termTier;

        return Term(termHash, termClassification, termIdf, termTier);
    }


    void TermMatchTreeConverter::ProcessNGramBuffer(RowMatchNode::Builder& builder,
                                                    RingBuffer<Term, c_log2MaxGramSize + 1>& gramBuffer)
    {
        NGramBuilder nGramBuilder;
        for (unsigned i = 0; i < gramBuffer.GetCount(); ++i)
        {
            nGramBuilder.AddGram(gramBuffer[i].GetRawHash(), gramBuffer[i].GetIdfSum());

            // Ngrams get a tier hint based on if the phrase is common or not.
            // Common phrases are DDR, rest are SSD.
            // Experimental phrases are always HDD.
            const Tier tierHint = nGramBuilder.GetTierHintOfRegularNGram(gramBuffer[i].GetClassification(),
                                                                         m_index.GetCommonPhrases());

            AppendTermRows(builder, Term(nGramBuilder, gramBuffer[i].GetClassification(), tierHint));

            if (m_generateNonBodyPlan && gramBuffer[i].GetClassification() == Stream::Full)
            {
                AppendTermRows(builder, Term(nGramBuilder, Stream::NonBody, tierHint));
            }
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
            // No space in IPlanRows for processing a fact TermMatchNode in the tree.
            LogB(Logging::Warning,
                 "IndexServe",
                 "Row count limit reached for fact with FactHandle value of %u "
                 "when processing a TermMatchNode",
                 fact);
        }
    }
}
