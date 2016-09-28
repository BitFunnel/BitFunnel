#pragma once


namespace BitFunnel
{

    // class FalsePositiveEvaluationNode;
    class IAllocator;
    class ISimpleIndex;
    class RowPlan;
    class TermMatchNode;

    class TermPlanConverter
    {
    public:
        static const RowPlan& BuildRowPlan(const TermMatchNode& termMatchNode,
                                           const ISimpleIndex& index,
                                           // bool generateNonBodyPlan,
                                           IAllocator& allocator);

        // static FalsePositiveEvaluationNode const & BuildFalsePositiveEvaluationPlan(
        //         TermMatchNode const & termMatchNode,
        //         IAllocator& allocator);
    };
}
