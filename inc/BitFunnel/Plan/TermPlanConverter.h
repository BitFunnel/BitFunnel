#pragma once


namespace BitFunnel
{
    namespace Allocators
    {
        class IAllocator;
    }

    class FalsePositiveEvaluationNode;
    class IIndexConfiguration;
    class RowPlan;
    class TermMatchNode;

    class TermPlanConverter
    {
    public:
        static const RowPlan& BuildRowPlan(const TermMatchNode& termMatchNode,
                                           const IIndexConfiguration& index,
                                           bool generateNonBodyPlan,
                                           Allocators::IAllocator& allocator);

        static FalsePositiveEvaluationNode const & BuildFalsePositiveEvaluationPlan(
                TermMatchNode const & termMatchNode,
                Allocators::IAllocator& allocator);
    };
}
