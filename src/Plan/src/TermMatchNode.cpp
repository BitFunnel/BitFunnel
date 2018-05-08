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

#include <cstring>  // For strcmp.

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Utilities/IObjectFormatter.h"
#include "BitFunnel/Utilities/IObjectParser.h"
#include "LoggerInterfaces/Logging.h"
#include "ObjectFormatter.h"
#include "PlanNodeUtilities.h"
#include "StringVector.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace Factories
    {
        TermMatchNode const &
            CreateTermMatchNode(std::istream& input,
                                IAllocator& allocator);
    }

    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************
    TermMatchNode const &
        Factories::CreateTermMatchNode(std::istream& input,
                                       IAllocator& allocator)
    {
        TextObjectParser parser(input, allocator, TermMatchNode::GetType);
        return TermMatchNode::Parse(parser);
    }


    //*************************************************************************
    //
    // TermMatchNode
    //
    //*************************************************************************
    // These text constants correspond to the values of the enumeration
    // TermMatchNode::NodeType.
    static const char* const c_typenames[] = {
        // Match nodes
        "And",
        "Not",
        "Or",
        "Phrase",
        "Unigram",
        "Fact"
    };


    int TermMatchNode::GetTypeTag() const
    {
        return GetType();
    }


    char const * TermMatchNode::GetTypeName() const
    {
        NodeType type = GetType();
        LogAssertB(type < TypeCount, "Invalid type.");
        return c_typenames[type];
    }


    // TODO: REVIEW: Can this method return size_t to avoid static_cast?
    int TermMatchNode::GetType(char const * name)
    {
        if (name[0] == 0)
        {
            return Null;
        }
        else
        {
            for (size_t i = 0; i < sizeof(c_typenames) / sizeof(char const *); ++i)
            {
                if (strcmp(name, c_typenames[i]) == 0)
                {
                    return static_cast<int>(i);
                }
            }
        }

        return Invalid;
    }


    TermMatchNode const & TermMatchNode::Parse(IObjectParser& parser)
    {
        int nodeType = parser.ReadTypeTag();

        switch (nodeType)
        {
        case AndMatch:
            return And::Parse(parser);
        case NotMatch:
            return ParseNode<Not>(parser);
        case OrMatch:
            return Or::Parse(parser);
        case PhraseMatch:
            return ParseNode<Phrase>(parser);
        case UnigramMatch:
            return ParseNode<Unigram>(parser);
        case FactMatch:
            return ParseNode<Fact>(parser);
        default:
            LogAbortB("Invalid node type.");
        }

        // Should never reach this line.
        BitFunnel::RecoverableError error("TermMatchNode::Parse: bad node type.");
        throw error;
    }


    //*************************************************************************
    //
    // TermMatchNode::And
    //
    //*************************************************************************
    char const * TermMatchNode::And::c_childrenFieldName = "Children";


    TermMatchNode::And::And(TermMatchNode const & left,
                            TermMatchNode const & right)
        : m_left(left),
          m_right(right)
    {
    }


    void TermMatchNode::And::Format(IObjectFormatter& formatter) const
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


    TermMatchNode::NodeType TermMatchNode::And::GetType() const
    {
        return TermMatchNode::AndMatch;
    }


    TermMatchNode const & TermMatchNode::And::GetLeft() const
    {
        return m_left;
    }


    TermMatchNode const & TermMatchNode::And::GetRight() const
    {
        return m_right;
    }


    TermMatchNode::And const & TermMatchNode::And::Parse(IObjectParser& parser)
    {
        parser.OpenObject();
        parser.OpenObjectField(c_childrenFieldName);

        parser.OpenList();

        // And nodes must have exactly two children.
        LogAssertB(parser.OpenListItem(), "OpenListItem() failed.");
        And const & node = dynamic_cast<And const &>(ParseList<TermMatchNode, And>(parser));

        parser.CloseList();
        parser.CloseObject();

        return node;
    }


    //*************************************************************************
    //
    // TermMatchNode::Not
    //
    //*************************************************************************
    char const * TermMatchNode::Not::c_childFieldName = "Child";


    TermMatchNode::Not::Not(TermMatchNode const & child)
        : m_child(child)
    {
        LogAssertB(child.GetType() != TermMatchNode::NotMatch,
                   "Expected TermMatchNode::NotMatch");
    }


    TermMatchNode::Not::Not(IObjectParser& parser)
        : m_child((parser.OpenObject(),
                   ParseNodeField<TermMatchNode>(parser, c_childFieldName)))
    {
        parser.CloseObject();
    }


    void TermMatchNode::Not::Format(IObjectFormatter& formatter) const
    {
        formatter.OpenObject(*this);
        formatter.OpenObjectField(c_childFieldName);
        GetChild().Format(formatter);
        formatter.CloseObject();
    }


    TermMatchNode::NodeType TermMatchNode::Not::GetType() const
    {
        return TermMatchNode::NotMatch;
    }


    TermMatchNode const & TermMatchNode::Not::GetChild() const
    {
        return m_child;
    }


    //*************************************************************************
    //
    // TermMatchNode::Or
    //
    //*************************************************************************
    char const * TermMatchNode::Or::c_childrenFieldName = "Children";


    TermMatchNode::Or::Or(TermMatchNode const & left,
                          TermMatchNode const & right)
        : m_left(left),
          m_right(right)
    {
    }


    void TermMatchNode::Or::Format(IObjectFormatter& formatter) const
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


    TermMatchNode::NodeType TermMatchNode::Or::GetType() const
    {
        return TermMatchNode::OrMatch;
    }


    TermMatchNode const & TermMatchNode::Or::GetLeft() const
    {
        return m_left;
    }


    TermMatchNode const & TermMatchNode::Or::GetRight() const
    {
        return m_right;
    }


    TermMatchNode::Or const & TermMatchNode::Or::Parse(IObjectParser& parser)
    {
        parser.OpenObject();
        parser.OpenObjectField(c_childrenFieldName);

        parser.OpenList();

        // Or nodes must have exactly two children.
        LogAssertB(parser.OpenListItem(), "OpenListItem() failed.");
        Or const & node = dynamic_cast<Or const &>(ParseList<TermMatchNode, Or>(parser));

        parser.CloseList();
        parser.CloseObject();

        return node;
    }


    //*************************************************************************
    //
    // TermMatchNode::Phrase
    //
    //*************************************************************************
    char const * TermMatchNode::Phrase::c_streamIdFieldName = "StreamId";
    char const * TermMatchNode::Phrase::c_gramsFieldName = "Grams";


    TermMatchNode::Phrase::Phrase(StringVector const & grams,
                                  Term::StreamId streamId)
        : m_streamId(streamId),
          m_grams(grams)
    {
        LogAssertB(m_grams.GetSize() >= 2, "Phrase must have at least 2 terms.");
    }


    TermMatchNode::Phrase::Phrase(IObjectParser& parser)
        : m_streamId((parser.OpenObject(),
                      static_cast<Term::StreamId>(
                          ParseObjectField<uint64_t>(parser, c_streamIdFieldName)))),
          m_grams((parser.OpenObjectField(c_gramsFieldName),
                   StringVector::Parse(parser, c_defaultInitialCapacity)))
    {
        // Phrase nodes must have at least two Terms.
        LogAssertB(m_grams.GetSize() >= 2, "Phrase must have at least 2 terms.");
        parser.CloseObject();
    }


    void TermMatchNode::Phrase::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenObject(*this);

        formatter.OpenObjectField(c_streamIdFieldName);
        formatter.Format(static_cast<size_t>(m_streamId));

        formatter.OpenObjectField(c_gramsFieldName);
        m_grams.Format(formatter);

        formatter.CloseObject();
    }


    TermMatchNode::NodeType TermMatchNode::Phrase::GetType() const
    {
        return TermMatchNode::PhraseMatch;
    }


    Term::StreamId TermMatchNode::Phrase::GetStreamId() const
    {
        return m_streamId;
    }


    StringVector const & TermMatchNode::Phrase::GetGrams() const
    {
        return m_grams;
    }


    //*************************************************************************
    //
    // TermMatchNode::Unigram
    //
    //*************************************************************************
    TermMatchNode::Unigram::Unigram(char const * text,
                                    Term::StreamId streamId)
        : m_text(text),
          m_streamId(streamId)
    {
    }


    TermMatchNode::Unigram::Unigram(IObjectParser& parser)
        : m_text((parser.OpenPrimitive(""),
                  ParsePrimitiveItem<char const *>(parser))),
          m_streamId(static_cast<Term::StreamId>(ParsePrimitiveItem<uint64_t>(parser)))
    {
        parser.ClosePrimitive();
    }


    void TermMatchNode::Unigram::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenPrimitive(GetTypeName());

        formatter.OpenPrimitiveItem();
        formatter.FormatStringLiteral(m_text);

        formatter.OpenPrimitiveItem();
        formatter.Format(static_cast<size_t>(m_streamId));

        formatter.ClosePrimitive();
    }


    TermMatchNode::NodeType TermMatchNode::Unigram::GetType() const
    {
        return TermMatchNode::UnigramMatch;
    }


    char const * TermMatchNode::Unigram::GetText() const
    {
        return m_text;
    }


    Term::StreamId TermMatchNode::Unigram::GetStreamId() const
    {
        return m_streamId;
    }


    //*************************************************************************
    //
    // TermMatchNode::Fact
    //
    //*************************************************************************
    TermMatchNode::Fact::Fact(FactHandle fact)
        : m_fact(fact)
    {
    }


    TermMatchNode::Fact::Fact(IObjectParser& parser)
        : m_fact((parser.OpenPrimitive(""),
                  ParsePrimitiveItem<FactHandle>(parser)))
    {
        parser.ClosePrimitive();
    }


    FactHandle TermMatchNode::Fact::GetFact() const
    {
        return m_fact;
    }


    TermMatchNode::NodeType TermMatchNode::Fact::GetType() const
    {
        return TermMatchNode::FactMatch;
    }


    void TermMatchNode::Fact::Format(IObjectFormatter& formatter) const
    {
        // WARNING: Field format order must be consistent with the order the
        // fields are declared in the header file. The reason is that the
        // initializers in the constructor will parse the fields in declaration
        // order.

        formatter.OpenPrimitive(GetTypeName());

        formatter.OpenPrimitiveItem();
        formatter.Format(static_cast<size_t>(m_fact));

        formatter.ClosePrimitive();
    }


    //*************************************************************************
    //
    // TermMatchNode::Builder
    //
    //*************************************************************************
    TermMatchNode::Builder::Builder(TermMatchNode const & parent,
                                    IAllocator& allocator)
        : m_allocator(allocator),
          m_targetType(parent.GetType()),
          m_firstChild(nullptr),
          m_node(nullptr)
    {
        if (parent.GetType() == TermMatchNode::PhraseMatch
            || parent.GetType() == TermMatchNode::UnigramMatch)
        {
            m_firstChild = &parent;
        }
    }


    TermMatchNode::Builder::Builder(TermMatchNode::NodeType nodeType,
                                    IAllocator& allocator)
        : m_allocator(allocator),
          m_targetType(nodeType),
          m_firstChild(nullptr),
          m_node(nullptr)
    {
        LogAssertB(nodeType == TermMatchNode::AndMatch
                   || nodeType == TermMatchNode::NotMatch
                   || nodeType == TermMatchNode::OrMatch,
                   "Invalid node type");
    }


    void TermMatchNode::Builder::AddChild(TermMatchNode const * childNode)
    {
        switch (m_targetType)
        {
        case TermMatchNode::AndMatch:
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
        case TermMatchNode::NotMatch:
            LogAssertB(m_firstChild == nullptr, "Expected a child node.");
            if (childNode != nullptr)
            {
                if (childNode->GetType() == TermMatchNode::NotMatch)
                {
                    m_firstChild = &dynamic_cast<Not const &>(*childNode).GetChild();
                }
                else
                {
                    m_firstChild = new (m_allocator.Allocate(sizeof(Not)))
                                       Not(*childNode);
                }
            }
            break;
        case TermMatchNode::OrMatch:
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
            LogAbortB("Unknown node type.");
        };
    }


    TermMatchNode const * TermMatchNode::Builder::Complete()
    {
        if (m_node == nullptr)
        {
            m_node = m_firstChild;
        }

        return m_node;
    }


    TermMatchNode const *
    TermMatchNode::Builder::CreatePhraseNode(StringVector const & grams,
                                             Term::StreamId streamId,
                                             IAllocator& allocator)
    {
        return
            new (allocator.Allocate(sizeof(Phrase)))
            Phrase(grams, streamId);
    }


    TermMatchNode const *
    TermMatchNode::Builder::CreateUnigramNode(char const * text,
                                              Term::StreamId streamId,
                                              IAllocator& allocator)
    {
        return
            new (allocator.Allocate(sizeof(Unigram)))
            Unigram(text, streamId);
    }


    TermMatchNode const *
    TermMatchNode::Builder::CreateFactNode(FactHandle fact,
                                           IAllocator& allocator)
    {
        return new (allocator.Allocate(sizeof(Fact))) Fact(fact);
    }
}
