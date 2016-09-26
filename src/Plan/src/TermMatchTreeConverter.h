#pragma once

#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/RowMatchNodes.h"
// #include "BitFunnel/Stream.h"
#include "BitFunnel/TermMatchNodes.h"


namespace BitFunnel
{
    class IAllocator;
    class IIndexConfiguration;
    class PlanRows;
    template <typename T, unsigned LOG2_CAPACITY>
    class RingBuffer;
    class Term;


    class TermMatchTreeConverter : NonCopyable
    {
    public:
        TermMatchTreeConverter(const IIndexConfiguration& index,
                               PlanRows& planRows,
                               bool generateNonBodyPlan,
                               Allocators::IAllocator& allocator);

        const RowMatchNode& BuildRowMatchTree(const TermMatchNode& root);

    private:
        const RowMatchNode* BuildMatchTree(const TermMatchNode& node);
        const RowMatchNode* BuildMatchTree(const TermMatchNode::And& node);
        const RowMatchNode* BuildMatchTree(const TermMatchNode::Not& node);
        const RowMatchNode* BuildMatchTree(const TermMatchNode::Or& node);
        const RowMatchNode* BuildMatchTree(const TermMatchNode::Phrase& node);
        const RowMatchNode* BuildMatchTree(const TermMatchNode::Unigram& node);
        const RowMatchNode* BuildMatchTree(const TermMatchNode::Fact& node);

        // Builds a RowMatchNode for a soft-deleted document row. This row
        // excludes documents which are marked as soft-deleted, from matching.
        const RowMatchNode* BuildSoftDeletedMatchNode();

        const Term GetUnigramTerm(char const * text, char const * suffix, Classification classification) const;
        void ProcessNGramBuffer(RowMatchNode::Builder& builder,
                                RingBuffer<Term, c_log2MaxGramSize + 1>& termBuffer);
        void AppendTermRows(RowMatchNode::Builder& builder, const Term& term);
        void AppendTermRows(RowMatchNode::Builder& builder, const FactHandle& fact);

        Allocators::IAllocator& m_allocator;
        const IIndexedIdfTable& m_idfTable;
        PlanRows& m_planRows;

        // A flag to indicate if NonBodyQueryPlan is requested to be generated.
        // bool m_generateNonBodyPlan;
    };
}
