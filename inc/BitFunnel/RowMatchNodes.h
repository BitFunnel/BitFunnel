#pragma once

#include "BitFunnel/AbstractRow.h"    // Embeds AbstractRow.
#include "BitFunnel/RowPlan.h"        // Inherits from RowPlanBase.


namespace BitFunnel
{
    namespace Allocators
    {
        class IAllocator;
    }

    class IObjectParser;

    class RowMatchNode : public RowPlanBase
    {
    public:
        // Nodes
        class And;
        class Not;
        class Or;
        class Report;
        class Row;

        // Node builder.
        class Builder;

        //Static parsing methods.
        static RowMatchNode const & Parse(IObjectParser& parser);
        static RowMatchNode const * ParseNullable(IObjectParser& parser);
    };


    class RowMatchNode::And : public RowMatchNode
    {
    public:
        And(RowMatchNode const & left, RowMatchNode const & right);

        void Format(IObjectFormatter& formatter) const;

        NodeType GetType() const;

        RowMatchNode const & GetLeft() const;
        RowMatchNode const & GetRight() const;

        static And const & Parse(IObjectParser& parser);

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        RowMatchNode const & m_left;
        RowMatchNode const & m_right;

        static char const * c_childrenFieldName;
    };


    class RowMatchNode::Not : public RowMatchNode
    {
    public:
        Not(RowMatchNode const & child);
        Not(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;

        NodeType GetType() const;

        RowMatchNode const & GetChild() const;

    private:
        RowMatchNode const & m_child;

        static char const * c_childFieldName;
    };


    class RowMatchNode::Or : public RowMatchNode
    {
    public:
        Or(RowMatchNode const & left, RowMatchNode const & right);

        void Format(IObjectFormatter& formatter) const;

        NodeType GetType() const;

        RowMatchNode const & GetLeft() const;
        RowMatchNode const & GetRight() const;

        static Or const & Parse(IObjectParser& parser);

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        RowMatchNode const & m_left;
        RowMatchNode const & m_right;

        static char const * c_childrenFieldName;
    };


    class RowMatchNode::Report : public RowMatchNode
    {
    public:
        Report(RowMatchNode const * child);
        Report(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;

        NodeType GetType() const;

        RowMatchNode const * GetChild() const;

    private:
        RowMatchNode const * m_child;

        static char const * c_childFieldName;
    };


    class RowMatchNode::Row : public RowMatchNode
    {
    public:
        Row(AbstractRow const & row);
        Row(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;

        NodeType GetType() const;

        AbstractRow const & GetRow() const;

    private:
        const AbstractRow m_row;

        static char const * c_rowFieldName;
    };


    class RowMatchNode::Builder : NonCopyable
    {
    public:
        Builder(RowMatchNode const & parent,
                Allocators::IAllocator& allocator);

        Builder(RowMatchNode::NodeType nodeType,
                Allocators::IAllocator& allocator);

        void AddChild(RowMatchNode const * child);

        RowMatchNode const * Complete();

        static RowMatchNode const *
        CreateReportNode(RowMatchNode const * child,
                         Allocators::IAllocator& allocator);

        static RowMatchNode const *
        CreateRowNode(AbstractRow const & row,
                      Allocators::IAllocator& allocator);

    private:
        Allocators::IAllocator& m_allocator;
        RowMatchNode::NodeType m_targetType;
        RowMatchNode const * m_firstChild;
        RowMatchNode const * m_node;
    };
}
