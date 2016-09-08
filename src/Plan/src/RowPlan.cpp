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

#include <cstring>

#include "BitFunnel/IObjectFormatter.h"
#include "BitFunnel/IObjectParser.h"
#include "BitFunnel/RowMatchNode.h"
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
        LogAssertB(type < TypeCount, "");
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
            for (size_t i = 0;
                 i < sizeof(c_typenames) / sizeof(const char*);
                 ++i)
            {
                if (strcmp(name, c_typenames[i]) == 0)
                {
                    // TODO: fix sizes so we don't need to cast.
                    return static_cast<int>(i);
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
          m_planRows(*(static_cast<IPlanRows*>(nullptr)))
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
        LogAssertB(parser.ReadTypeTag() == Plan, "");
        return ParseNode<RowPlan>(parser);
    }
}
