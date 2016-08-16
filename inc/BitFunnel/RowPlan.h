#pragma once

#include "BitFunnel/IPersistableObject.h"     // Inherits from IPersistableObject.
#include "BitFunnel/NonCopyable.h"            // Inherits from NonCopyable.


namespace BitFunnel
{
    class IObjectFormatter;
    class IObjectParser;
    class IPlanRows;
    class RowMatchNode;


    class RowPlanBase : public IPersistableObject,
                        protected NonCopyable
    {
    public:
        enum NodeType
        {
            Invalid = -2,
            Null = -1,

            // DESIGN NOTE: legal node types have consecutive values starting
            // at zero so that TypeCount is equal to the number of legal nodes.

            // Match
            AndMatch = 0,
            NotMatch,
            OrMatch,
            ReportMatch,
            RowMatch,

            // Plan
            Plan,

            // Total number of node types.
            TypeCount
        };

        virtual NodeType GetType() const = 0;

        //
        // IPersistableObject methods
        //
        int GetTypeTag() const;
        const char* GetTypeName() const;

        //
        // Static methods
        //
        static int GetType(const char*);
    };


    class RowPlan : public RowPlanBase
    {
    public:
        RowPlan(RowMatchNode const & matchTree,
                IPlanRows const & planRows);

        RowPlan(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;

        RowMatchNode const & GetMatchTree() const;
        IPlanRows const & GetPlanRows() const;

        NodeType GetType() const;

        static RowPlan& Parse(IObjectParser& parser);

    private:
        // WARNING: The persistence format depends on the order in which the
        // following three members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the RowPlan::RowPlan()
        // and RowPlan::Format().
        RowMatchNode const & m_matchTree;
        IPlanRows const & m_planRows;
    };
}
