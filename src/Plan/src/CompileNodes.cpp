#include "BitFunnel/AbstractRow.h"
#include "CompileNodes.h"
#include "CompileNodeUtilities.h"
#include "BitFunnel/ICodeGenerator.h"
#include "BitFunnel/IObjectFormatter.h"
#include "BitFunnel/IObjectParser.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // CompileNode
    //
    //*************************************************************************
    static const char* const c_typenames[] = {
        // RankDown nodes.
        "AndRowJz",
        "LoadRowJz",
        "Or",
        "RankDown",
        "Report",

        // RankZero nodes.
        "AndTree",
        "LoadRow",
        "Not",
        "OrTree"
    };


    int CompileNode::GetTypeTag() const
    {
        return GetType();
    }


    const char* CompileNode::GetTypeName() const
    {
        NodeType type = GetType();
        LogAssertB(type < TypeCount, "type >= TypeCount");
        return c_typenames[type];
    }


    int CompileNode::GetType(const char* name)
    {
        if (name[0] == 0)
        {
            return Null;
        }
        else
        {
            for (unsigned i = 0; i < sizeof(c_typenames) / sizeof(const char*); ++i)
            {
                if (strcmp(name, c_typenames[i]) == 0)
                {
                    return i;
                }
            }
        }

        return Invalid;
    }


    CompileNode const & CompileNode::Parse(IObjectParser& parser)
    {
        CompileNode const * node = ParseNullable(parser);
        LogAssertB(node != nullptr, "parsing nullptr");
        return *node;
    }


    CompileNode const * CompileNode::ParseNullable(IObjectParser& parser)
    {
        int nodeType = parser.ReadTypeTag();

        switch (nodeType)
        {
        case opAndRowJz:
            return &ParseNode<AndRowJz>(parser);
            break;
        case opLoadRowJz:
            return &ParseNode<LoadRowJz>(parser);
            break;
        case opOr:
            return &Or::Parse(parser);
            break;
        case opRankDown:
            return &ParseNode<RankDown>(parser);
            break;
        case opReport:
            return &ParseNode<Report>(parser);
            break;

        case opAndTree:
            return &AndTree::Parse(parser);
            break;
        case opLoadRow:
            return &ParseNode<LoadRow>(parser);
            break;
        case opNot:
            return &ParseNode<Not>(parser);
            break;
        case opOrTree:
            return &OrTree::Parse(parser);
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
    // CompileNode::Binary
    //
    //*************************************************************************
    const char* CompileNode::Binary::c_childrenFieldName = "Children";


    CompileNode::Binary::Binary(CompileNode const & left, CompileNode const & right)
        : m_left(left),
          m_right(right)
    {
    }


    void CompileNode::Binary::Format(IObjectFormatter& formatter) const
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


    void CompileNode::Binary::Compile(ICodeGenerator & /*codeGenerator*/) const
    {
    }


    CompileNode const & CompileNode::Binary::GetLeft() const
    {
        return m_left;
    }


    CompileNode const & CompileNode::Binary::GetRight() const
    {
        return m_right;
    }


    //*************************************************************************
    //
    // CompileNode::AndRowJz
    //
    //*************************************************************************
    const char* CompileNode::AndRowJz::c_rowFieldName = "Row";
    const char* CompileNode::AndRowJz::c_childFieldName = "Child";


    CompileNode::AndRowJz::AndRowJz(AbstractRow const & row, CompileNode const & child)
        : m_row(row),
          m_child(child)
    {
    }


    CompileNode::AndRowJz::AndRowJz(IObjectParser& parser)
        : m_row((parser.OpenObject(),
                 ParseObjectField<AbstractRow>(parser, c_rowFieldName))),
          m_child(ParseNodeField<CompileNode>(parser, c_childFieldName))
    {
        parser.CloseObject();
    }


    void CompileNode::AndRowJz::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenObject(*this);

        formatter.OpenObjectField(c_rowFieldName);
        m_row.Format(formatter, nullptr);

        formatter.OpenObjectField(c_childFieldName);
        m_child.Format(formatter);

        formatter.CloseObject();
    }


    void CompileNode::AndRowJz::Compile(ICodeGenerator & code) const
    {
        code.AndRow(m_row.GetId(), m_row.IsInverted(), m_row.GetRankDelta());
        ICodeGenerator::Label label = code.AllocateLabel();
        code.Jz(label);
        m_child.Compile(code);
        code.PlaceLabel(label);
    }


    CompileNode::NodeType CompileNode::AndRowJz::GetType() const
    {
        return CompileNode::opAndRowJz;
    }


    AbstractRow const & CompileNode::AndRowJz::GetRow() const
    {
        return m_row;
    }


    CompileNode const & CompileNode::AndRowJz::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // CompileNode::LoadRowJz
    //
    //*************************************************************************
    const char* CompileNode::LoadRowJz::c_rowFieldName = "Row";
    const char* CompileNode::LoadRowJz::c_childFieldName = "Child";


    CompileNode::LoadRowJz::LoadRowJz(AbstractRow const & row, CompileNode const & child)
        : m_row(row),
          m_child(child)
    {
    }


    CompileNode::LoadRowJz::LoadRowJz(IObjectParser& parser)
        : m_row((parser.OpenObject(),
                 ParseObjectField<AbstractRow>(parser, c_rowFieldName))),
          m_child(ParseNodeField<CompileNode>(parser, c_childFieldName))
    {
        parser.CloseObject();
    }


    void CompileNode::LoadRowJz::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenObject(*this);

        formatter.OpenObjectField(c_rowFieldName);
        m_row.Format(formatter, nullptr);

        formatter.OpenObjectField(c_childFieldName);
        m_child.Format(formatter);

        formatter.CloseObject();
    }


    void CompileNode::LoadRowJz::Compile(ICodeGenerator & code) const
    {
        code.LoadRow(m_row.GetId(), m_row.IsInverted(), m_row.GetRankDelta());
        ICodeGenerator::Label label = code.AllocateLabel();
        code.Jz(label);
        m_child.Compile(code);
        code.PlaceLabel(label);
    }


    CompileNode::NodeType CompileNode::LoadRowJz::GetType() const
    {
        return CompileNode::opLoadRowJz;
    }


    AbstractRow const & CompileNode::LoadRowJz::GetRow() const
    {
        return m_row;
    }


    CompileNode const & CompileNode::LoadRowJz::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // CompileNode::Or
    //
    //*************************************************************************
    CompileNode::Or::Or(CompileNode const & left, CompileNode const & right)
        : Binary(left, right)
    {
    }


    void CompileNode::Or::Compile(ICodeGenerator & code) const
    {
        code.Push();
        GetLeft().Compile(code);
        code.Pop();
        GetRight().Compile(code);
    }


    CompileNode::NodeType CompileNode::Or::GetType() const
    {
        return CompileNode::opOr;
    }


    CompileNode::Or const & CompileNode::Or::Parse(IObjectParser& parser)
    {
        return ParseBinaryTree<Or>(parser, c_childrenFieldName);
    }


    //*************************************************************************
    //
    // CompileNode::RankDown
    //
    //*************************************************************************
    char const * CompileNode::RankDown::c_deltaFieldName = "Delta";
    char const * CompileNode::RankDown::c_childFieldName = "Child";


    CompileNode::RankDown::RankDown(Rank delta, CompileNode const & child)
        : m_delta(delta),
          m_child(child)
    {
    }


    CompileNode::RankDown::RankDown(IObjectParser& parser)
        : m_delta((parser.OpenObject(),
                   ParseObjectField<unsigned>(parser, c_deltaFieldName))),
          m_child(ParseNodeField<CompileNode>(parser, c_childFieldName))
    {
        parser.CloseObject();
    }


    void CompileNode::RankDown::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenObject(*this);

        formatter.OpenObjectField(c_deltaFieldName);
        formatter.Format(m_delta);

        formatter.OpenObjectField(c_childFieldName);
        m_child.Format(formatter);

        formatter.CloseObject();
    }


    void CompileNode::RankDown::Compile(ICodeGenerator & code) const
    {
        code.LeftShiftOffset(m_delta);
        ICodeGenerator::Label label0 = code.AllocateLabel();

        unsigned iterations = (1 << m_delta) - 1;
        for (unsigned i = 0; i < iterations; ++i)
        {
            code.Push();
            code.Call(label0);
            code.Pop();
            code.IncrementOffset();
        }
        code.Call(label0);
        ICodeGenerator::Label label1 = code.AllocateLabel();
        code.Jmp(label1);
        code.PlaceLabel(label0);
        m_child.Compile(code);
        code.Return();
        code.PlaceLabel(label1);
        code.RightShiftOffset(m_delta);
    }


    CompileNode::NodeType CompileNode::RankDown::GetType() const
    {
        return CompileNode::opRankDown;
    }


    Rank CompileNode::RankDown::GetDelta() const
    {
        return m_delta;
    }


    CompileNode const & CompileNode::RankDown::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // CompileNode::Report
    //
    //*************************************************************************
    const char* CompileNode::Report::c_childFieldName = "Child";


    CompileNode::Report::Report(CompileNode const * child)
        : m_child(child)
    {
    }


    CompileNode::Report::Report(IObjectParser& parser)
        : m_child((parser.OpenObject(),
                   ParseNullableNodeField<CompileNode>(parser, c_childFieldName)))
    {
        parser.CloseObject();
    }


    void CompileNode::Report::Format(IObjectFormatter& formatter) const
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


    void CompileNode::Report::Compile(ICodeGenerator & code) const
    {
        if (m_child == nullptr)
        {
            code.Report();
        }
        else
        {
            code.Push();
            m_child->Compile(code);
            code.AndStack();
            ICodeGenerator::Label label = code.AllocateLabel();
            code.Jz(label);
            code.Report();
            code.PlaceLabel(label);
        }
    }


    CompileNode::NodeType CompileNode::Report::GetType() const
    {
        return CompileNode::opReport;
    }


    CompileNode const * CompileNode::Report::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // CompileNode::AndTree
    //
    //*************************************************************************
    CompileNode::AndTree::AndTree(CompileNode const & left,
                                  CompileNode const & right)
        : Binary(left, right)
    {
    }


    void CompileNode::AndTree::Compile(ICodeGenerator & code) const
    {
        GetLeft().Compile(code);
        ICodeGenerator::Label label = code.AllocateLabel();

        // UpdateFlags() call is needed because Compile() does not
        // guarantee the 'Z' flag refects the state of the accumulator.
        code.UpdateFlags();
        code.Jz(label);
        code.Push();
        GetRight().Compile(code);
        code.AndStack();
        code.PlaceLabel(label);
    }


    CompileNode::NodeType CompileNode::AndTree::GetType() const
    {
        return CompileNode::opAndTree;
    }


    CompileNode::AndTree const & CompileNode::AndTree::Parse(IObjectParser& parser)
    {
        return ParseBinaryTree<AndTree>(parser, c_childrenFieldName);
    }


    //*************************************************************************
    //
    // CompileNode::LoadRow
    //
    //*************************************************************************
    const char* CompileNode::LoadRow::c_rowFieldName = "Row";


    CompileNode::LoadRow::LoadRow(AbstractRow const & row)
        : m_row(row)
    {
    }


    CompileNode::LoadRow::LoadRow(IObjectParser& parser)
        : m_row((parser.OpenPrimitive(""),
                 AbstractRow(parser, true)))
    {
        parser.ClosePrimitive();
    }


    void CompileNode::LoadRow::Format(IObjectFormatter& formatter) const
    {
        m_row.Format(formatter, GetTypeName());
    }


    void CompileNode::LoadRow::Compile(ICodeGenerator & code) const
    {
        code.LoadRow(m_row.GetId(), m_row.IsInverted(), m_row.GetRankDelta());
    }


    CompileNode::NodeType CompileNode::LoadRow::GetType() const
    {
        return CompileNode::opLoadRow;
    }


    AbstractRow const & CompileNode::LoadRow::GetRow() const
    {
        return m_row;
    }


    //*************************************************************************
    //
    // CompileNode::Not
    //
    //*************************************************************************
    const char* CompileNode::Not::c_childFieldName = "Child";


    CompileNode::Not::Not(CompileNode const & child)
        : m_child(child)
    {
    }


    CompileNode::Not::Not(IObjectParser& parser)
        : m_child((parser.OpenObject(),
                   ParseNodeField<CompileNode>(parser, c_childFieldName)))
    {
        parser.CloseObject();
    }


    void CompileNode::Not::Format(IObjectFormatter& formatter) const
    {
        formatter.OpenObject(*this);
        formatter.OpenObjectField(c_childFieldName);
        m_child.Format(formatter);
        formatter.CloseObject();
    }


    void CompileNode::Not::Compile(ICodeGenerator & code) const
    {
        GetChild().Compile(code);
        code.Not();
    }


    CompileNode::NodeType CompileNode::Not::GetType() const
    {
        return CompileNode::opNot;
    }


    CompileNode const & CompileNode::Not::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // CompileNode::OrTree
    //
    //*************************************************************************
    CompileNode::OrTree::OrTree(CompileNode const & left,
                                CompileNode const & right)
        : Binary(left, right)
    {
    }


    void CompileNode::OrTree::Compile(ICodeGenerator & code) const
    {
        GetLeft().Compile(code);
        code.Push();
        GetRight().Compile(code);
        code.OrStack();
    }


    CompileNode::NodeType CompileNode::OrTree::GetType() const
    {
        return CompileNode::opOrTree;
    }


    CompileNode::OrTree const & CompileNode::OrTree::Parse(IObjectParser& parser)
    {
        return ParseBinaryTree<OrTree>(parser, c_childrenFieldName);
    }
}
