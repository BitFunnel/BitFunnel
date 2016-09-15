#pragma once

#include "BitFunnel/NonCopyable.h"            // Inherits from NonCopyable.


namespace BitFunnel
{
    class IScoringEngine;
    class QueryPreferences;
    class TermMatchNode;  


    class TermPlan : NonCopyable
    {
    public:
        TermPlan(TermMatchNode const & matchTree,
                 IScoringEngine const & scoringEngine,
                 QueryPreferences const & queryPreferences);

        TermMatchNode const & GetMatchTree() const;
        IScoringEngine const & GetScoringEngine() const;
        QueryPreferences const & GetQueryPreferences() const;

    private:
        // WARNING: TermPlan is designed to be allocated by an arena
        // allocator, so this destructor will never be called. Therefore,
        // TermPlan should hold no resources other than memory from
        // the arena allocator.
        ~TermPlan();


        // WARNING: The persistence format depends on the order in which the
        // following three members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the TermPlan::TermPlan()
        // and TermPlan::Format().
        TermMatchNode const & m_matchTree;
        IScoringEngine const & m_scoringEngine;
        QueryPreferences const & queryPreferences;
    };
}
