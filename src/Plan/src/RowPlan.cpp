#include "stdafx.h"

#include "BitFunnel/IObjectFormatter.h"
#include "BitFunnel/IObjectParser.h"
#include "BitFunnel/RowMatchNodes.h"
#include "LoggerInterfaces/Logging.h"
#include "PlanNodeUtilities.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // RowPlanNode
    //
    //*************************************************************************
    static const char* const c_typenames[] = {
        // Match nodes
        "And",
        "Not",
        "Or",
        "Report",
        "Row",

        // Plan
        "RowPlan"
    };


    int RowPlanBase::GetTypeTag() const
    {
        return GetType();
    }


    const char* RowPlanBase::GetTypeName() const
    {
        NodeType type = GetType();
        LogAssertB(type < TypeCount);
        return c_typenames[type];
    }


    int RowPlanBase::GetType(const char* name)
    {
        if (name[0] == 0)
        {
            return Null;
        }
        else
        {
            for (int i = 0; i < sizeof(c_typenames) / sizeof(const char*); ++i)
            {
                if (strcmp(name, c_typenames[i]) == 0)
                {
                    return i;
                }
            }
        }

        return Invalid;
    }


    //*************************************************************************
    //
    // RowPlan
    //
    //*************************************************************************
    static const char* c_matchTreeField = "Match";

    RowPlan::RowPlan(RowMatchNode const & matchTree,
                     IPlanRows const & planRows)
        : m_matchTree(matchTree),
          m_planRows(planRows)
    {
    }


    // TODO We should investigate whether we are still using PlanRows inside the 
    // RowPlan. It is possible that we are using them inside the RowPlan, but 
    // never wrote the deserializing constructor because never transported 
    // RowPlans between machines. TFS 490813.
    RowPlan::RowPlan(IObjectParser& parser)
        : m_matchTree((parser.OpenObject(),
                       ParseNodeField<RowMatchNode>(parser, c_matchTreeField))),
          m_planRows(*(IPlanRows*)nullptr)
    {
        parser.CloseObject();
    }


    void RowPlan::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenObject(*this);

        formatter.OpenObjectField(c_matchTreeField);
        m_matchTree.Format(formatter);

        formatter.CloseObject();
    }


    RowMatchNode const & RowPlan::GetMatchTree() const
    {
        return m_matchTree;
    }


    IPlanRows const & RowPlan::GetPlanRows() const
    {
        return m_planRows;
    }


    RowPlanBase::NodeType RowPlan::GetType() const
    {
        return RowPlanBase::Plan;
    }


    RowPlan& RowPlan::Parse(IObjectParser& parser)
    {
        LogAssertB(parser.ReadTypeTag() == Plan);
        return ParseNode<RowPlan>(parser);
    }
}
