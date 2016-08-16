#include "stdafx.h"

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "BitFunnel/AbstractRow.h"
#include "BitFunnel/IObjectFormatter.h"
#include "BitFunnel/IObjectParser.h"
#include "LoggerInterfaces/Logging.h"
#include "ObjectFormatter.h"
#include "PlanNodeUtilities.h"
#include "BitFunnel/RowMatchNodes.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // RowMatchNode
    //
    //*************************************************************************
    RowMatchNode const & RowMatchNode::Parse(IObjectParser& parser)
    {
        RowMatchNode const * node = ParseNullable(parser);
        LogAssertB(node != nullptr);
        return *node;
    }


    RowMatchNode const * RowMatchNode::ParseNullable(IObjectParser& parser)
    {
        int nodeType = parser.ReadTypeTag();

        switch (nodeType)
        {
        case AndMatch:
            return &And::Parse(parser);
            break;
        case ReportMatch:
            return &ParseNode<Report>(parser);
            break;
        case RowMatch:
            return &ParseNode<Row>(parser);
            break;
        case NotMatch:
            return &ParseNode<Not>(parser);
            break;
        case OrMatch:
            return &Or::Parse(parser);
            break;
        case Null:
            return nullptr;
        default:
            LogAbortB("Invalid node type.");
        }

        return nullptr;
    }


    //*************************************************************************
    //
    // RowMatchNode::And
    //
    //*************************************************************************
    const char* RowMatchNode::And::c_childrenFieldName = "Children";


    RowMatchNode::And::And(RowMatchNode const & left,
                           RowMatchNode const & right)
        : m_left(left),
          m_right(right)
    {
    }


    void RowMatchNode::And::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenObject(*this);

        formatter.OpenObjectField(c_childrenFieldName);
        formatter.OpenList();

        FormatList(*this, formatter);

        formatter.CloseList();
        formatter.CloseObject();
    }


    RowMatchNode::NodeType RowMatchNode::And::GetType() const
    {
        return RowMatchNode::AndMatch;
    }


    RowMatchNode const & RowMatchNode::And::GetLeft() const
    {
        return m_left;
    }


    RowMatchNode const & RowMatchNode::And::GetRight() const
    {
        return m_right;
    }


    RowMatchNode::And const & RowMatchNode::And::Parse(IObjectParser& parser)
    {
        parser.OpenObject();
        parser.OpenObjectField(c_childrenFieldName);

        parser.OpenList();

        // And nodes must have exactly two children.
        LogAssertB(parser.OpenListItem());
        And const & node = dynamic_cast<And const &>(ParseList<RowMatchNode, And>(parser));

        parser.CloseList();
        parser.CloseObject();

        return node;
    }


    //*************************************************************************
    //
    // RowMatchNode::Not
    //
    //*************************************************************************
    const char* RowMatchNode::Not::c_childFieldName = "Child";


    RowMatchNode::Not::Not(RowMatchNode const & child)
        : m_child(child)
    {
        LogAssertB(child.GetType() != RowMatchNode::NotMatch);
    }


    RowMatchNode::Not::Not(IObjectParser& parser)
        : m_child((parser.OpenObject(),
                   ParseNodeField<RowMatchNode>(parser, c_childFieldName)))
    {
        parser.CloseObject();
    }


    void RowMatchNode::Not::Format(IObjectFormatter& formatter) const
    {
        formatter.OpenObject(*this);
        formatter.OpenObjectField(c_childFieldName);
        GetChild().Format(formatter);
        formatter.CloseObject();
    }


    RowMatchNode::NodeType RowMatchNode::Not::GetType() const
    {
        return RowMatchNode::NotMatch;
    }


    RowMatchNode const & RowMatchNode::Not::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // RowMatchNode::Or
    //
    //*************************************************************************
    const char* RowMatchNode::Or::c_childrenFieldName = "Children";


    RowMatchNode::Or::Or(RowMatchNode const & left, RowMatchNode const & right)
        : m_left(left),
          m_right(right)
    {
    }


    void RowMatchNode::Or::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenObject(*this);

        formatter.OpenObjectField(c_childrenFieldName);
        formatter.OpenList();

        FormatList(*this, formatter);

        formatter.CloseList();
        formatter.CloseObject();
    }


    RowMatchNode::NodeType RowMatchNode::Or::GetType() const
    {
        return RowMatchNode::OrMatch;
    }


    RowMatchNode const & RowMatchNode::Or::GetLeft() const
    {
        return m_left;
    }


    RowMatchNode const & RowMatchNode::Or::GetRight() const
    {
        return m_right;
    }


    RowMatchNode::Or const & RowMatchNode::Or::Parse(IObjectParser& parser)
    {
        parser.OpenObject();
        parser.OpenObjectField(c_childrenFieldName);

        parser.OpenList();

        // Or nodes must have exactly two children.
        LogAssertB(parser.OpenListItem());
        Or const & node = dynamic_cast<Or const &>(ParseList<RowMatchNode, Or>(parser));

        parser.CloseList();
        parser.CloseObject();

        return node;
    }


    //*************************************************************************
    //
    // RowMatchNode::Report
    //
    //*************************************************************************
    const char* RowMatchNode::Report::c_childFieldName = "Child";


    RowMatchNode::Report::Report(RowMatchNode const * child)
        : m_child(child)
    {
    }


    RowMatchNode::Report::Report(IObjectParser& parser)
        : m_child((parser.OpenObject(),
                   ParseNullableNodeField<RowMatchNode>(parser, c_childFieldName)))
    {
        parser.CloseObject();
    }


    void RowMatchNode::Report::Format(IObjectFormatter& formatter) const
    {
        formatter.OpenObject(*this);
        formatter.OpenObjectField(c_childFieldName);
        if (m_child == nullptr)
        {
            formatter.NullObject();
        }
        else
        {
            m_child->Format(formatter);
        }
        formatter.CloseObject();
    }


    RowMatchNode::NodeType RowMatchNode::Report::GetType() const
    {
        return RowMatchNode::ReportMatch;
    }


    RowMatchNode const * RowMatchNode::Report::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // RowMatchNode::Row
    //
    //*************************************************************************
    const char* RowMatchNode::Row::c_rowFieldName = "Row";


    RowMatchNode::Row::Row(AbstractRow const & row)
        : m_row(row)
    {
    }


    RowMatchNode::Row::Row(IObjectParser& parser)
        : m_row((parser.OpenPrimitive(""),
                 AbstractRow(parser, true)))
    {
        parser.ClosePrimitive();
    }


    void RowMatchNode::Row::Format(IObjectFormatter& formatter) const
    {
        m_row.Format(formatter, nullptr);
    }


    RowMatchNode::NodeType RowMatchNode::Row::GetType() const
    {
        return RowMatch;
    }


    AbstractRow const & RowMatchNode::Row::GetRow() const
    {
        return m_row;
    }


    //*************************************************************************
    //
    // RowMatchNode::Builder
    //
    //*************************************************************************
    RowMatchNode::Builder::Builder(RowMatchNode const & parent,
                                   Allocators::IAllocator& allocator)
        : m_allocator(allocator),
          m_targetType(parent.GetType()),
          m_firstChild(nullptr),
          m_node(nullptr)
    {
        if (parent.GetType() == RowMatchNode::RowMatch)
        {
            m_firstChild = &parent;
        }
    }


    RowMatchNode::Builder::Builder(RowMatchNode::NodeType nodeType,
                                   Allocators::IAllocator& allocator)
        : m_allocator(allocator),
          m_targetType(nodeType),
          m_firstChild(nullptr),
          m_node(nullptr)
    {
        LogAssertB(nodeType == RowMatchNode::AndMatch
                   || nodeType == RowMatchNode::NotMatch
                   || nodeType == RowMatchNode::OrMatch);
    }


    void RowMatchNode::Builder::AddChild(RowMatchNode const * childNode)
    {
        switch (m_targetType)
        {
        case AndMatch:
            if (m_firstChild == nullptr)
            {
                m_firstChild = childNode;
            }
            else if (childNode != nullptr)
            {
                if (m_node == nullptr)
                {
                    m_node = new (m_allocator.Allocate(sizeof(And)))
                                 And(*childNode, *m_firstChild);
                }
                else
                {
                    m_node = new (m_allocator.Allocate(sizeof(And)))
                                 And(*childNode, *m_node);
                }
            }
            break;
        case NotMatch:
            LogAssertB(m_firstChild == nullptr);
            if (childNode != nullptr)
            {
                if (childNode->GetType() == NotMatch)
                {
                    m_firstChild = &dynamic_cast<Not const &>(*childNode).GetChild();
                }
                else if (childNode->GetType() == RowMatch)
                {
                    AbstractRow const & row = dynamic_cast<Row const &>(*childNode).GetRow();
                    m_firstChild = new (m_allocator.Allocate(sizeof(Row)))
                                       Row(AbstractRow(row.GetId(), row.GetRank(), !row.IsInverted()));
                }
                else
                {
                    m_firstChild = new (m_allocator.Allocate(sizeof(Not)))
                                       Not(*childNode);
                }
            }
            break;
        case OrMatch:
            if (m_firstChild == nullptr)
            {
                m_firstChild = childNode;
            }
            else if (childNode != nullptr)
            {
                if (m_node == nullptr)
                {
                    m_node = new (m_allocator.Allocate(sizeof(Or)))
                                 Or(*childNode, *m_firstChild);
                }
                else
                {
                    m_node = new (m_allocator.Allocate(sizeof(Or)))
                                 Or(*childNode, *m_node);
                }
            }
            break;
        default:
            LogAbortB();
        };
    }


    RowMatchNode const * RowMatchNode::Builder::Complete()
    {
        if (m_node == nullptr)
        {
            m_node = m_firstChild;
        }

        return m_node;
    }


    RowMatchNode const *
    RowMatchNode::Builder::CreateReportNode(RowMatchNode const * child,
                                            Allocators::IAllocator& allocator)
    {
        return new (allocator.Allocate(sizeof(Report))) Report(child);
    }


    RowMatchNode const *
    RowMatchNode::Builder::CreateRowNode(AbstractRow const & row,
                                         Allocators::IAllocator& allocator)
    {
        return new (allocator.Allocate(sizeof(Row))) Row(row);
    }
}
