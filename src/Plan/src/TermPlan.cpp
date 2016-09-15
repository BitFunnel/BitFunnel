#include "stdafx.h"

#include "BitFunnel/TermPlan.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermPlan
    //
    //*************************************************************************
    TermPlan::TermPlan(TermMatchNode const & matchTree,
                       IScoringEngine const & scoringEngine,
                       QueryPreferences const & queryPreferences)
        : m_matchTree(matchTree),
          m_scoringEngine(scoringEngine),
          queryPreferences(queryPreferences)
    {
    }


    TermPlan::~TermPlan()
    {
        // WARNING: TermPlan is designed to be allocated by an arena
        // allocator, so this destructor will never be called. Therefore,
        // TermPlan should hold no resources other than memory from
        // the arena allocator.
    }


    TermMatchNode const & TermPlan::GetMatchTree() const
    {
        return m_matchTree;
    }


    IScoringEngine const & TermPlan::GetScoringEngine() const
    {
        return m_scoringEngine;
    }


    QueryPreferences const & TermPlan::GetQueryPreferences() const
    {
        return queryPreferences;
    }
}
