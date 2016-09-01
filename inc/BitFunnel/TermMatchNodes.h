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

#pragma once

#include "BitFunnel/Index/IFactSet.h"       // Embeds FactHandle.
#include "BitFunnel/Classification.h"       // Embeds Classification.
#include "BitFunnel/IPersistableObject.h"   // Inherits from IPersistableObject.
#include "BitFunnel/NonCopyable.h"          // Inherits from NonCopyable.

namespace BitFunnel
{
    class IAllocator;
    class IObjectFormatter;
    class IObjectParser;
    class StringVector;


    class TermMatchNode : public IPersistableObject,
                          protected NonCopyable
    {
    public:
        virtual ~TermMatchNode() {};

        enum NodeType
        {
            Invalid = -2,
            Null = -1,

            // DESIGN NOTE: legal node types have consecutive values starting
            // at zero so that TypeCount is equal to the number of legal nodes.
            // Changes in this enum must also be reflected in c_typenames[] in 
            // TermMatchNodes.cpp.

            // Match
            AndMatch = 0,
            NotMatch,
            OrMatch,
            PhraseMatch,
            UnigramMatch,
            FactMatch,
            TypeCount
        };

        virtual NodeType GetType() const = 0;

        //
        // IPersistableObject methods
        //
        int GetTypeTag() const;
        char const * GetTypeName() const;

        //
        // Static methods
        //
        static int GetType(char const *);

        // Nodes
        class And;
        class Not;
        class Or;
        class Phrase;
        class Unigram;
        class Fact;

        // Node builder.
        class Builder;

        //Static parsing methods.
        static TermMatchNode const & Parse(IObjectParser& parser);
    };


    class TermMatchNode::And : public TermMatchNode
    {
    public:
        And(TermMatchNode const & left, TermMatchNode const & right);

        //
        // IPersistableObject methods via TermMatchNode.
        //
        virtual void Format(IObjectFormatter& formatter) const override;

        //
        // TermMatchNode methods.
        //
        virtual NodeType GetType() const override;

        TermMatchNode const & GetLeft() const;
        TermMatchNode const & GetRight() const;

        static And const & Parse(IObjectParser& parser);

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        TermMatchNode const & m_left;
        TermMatchNode const & m_right;

        static char const * c_childrenFieldName;
    };


    class TermMatchNode::Not : public TermMatchNode
    {
    public:
        Not(TermMatchNode const & child);
        Not(IObjectParser& parser);

        //
        // IPersistableObject methods via TermMatchNode.
        //
        virtual void Format(IObjectFormatter& formatter) const override;

        //
        // TermMatchNode methods.
        //
        virtual NodeType GetType() const override;

        TermMatchNode const & GetChild() const;

    private:
        TermMatchNode const & m_child;

        static char const * c_childFieldName;
    };


    class TermMatchNode::Or : public TermMatchNode
    {
    public:
        Or(TermMatchNode const & left, TermMatchNode const & right);

        //
        // IPersistableObject methods via TermMatchNode.
        //
        virtual void Format(IObjectFormatter& formatter) const override;

        //
        // TermMatchNode methods.
        //
        virtual NodeType GetType() const override;

        TermMatchNode const & GetLeft() const;
        TermMatchNode const & GetRight() const;

        static Or const & Parse(IObjectParser& parser);

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        TermMatchNode const & m_left;
        TermMatchNode const & m_right;

        static char const * c_childrenFieldName;
    };


    class TermMatchNode::Phrase : public TermMatchNode
    {
    public:
        Phrase(StringVector const & grams, char const * suffix, Classification classification);
        Phrase(IObjectParser& parser);

        //
        // IPersistableObject methods via TermMatchNode.
        //
        virtual void Format(IObjectFormatter& formatter) const override;

        //
        // TermMatchNode methods.
        //
        virtual NodeType GetType() const override;

        Classification GetClassification() const;
        StringVector const & GetGrams() const;
        char const * GetSuffix() const;

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        Classification const m_classification;
        char const * const m_suffix;
        StringVector const & m_grams;

        static char const * c_classificationFieldName;
        static char const * c_gramsFieldName;
        static char const * c_suffixFieldName;
        static const unsigned c_defaultInitialCapacity = 10;
    };


    class TermMatchNode::Unigram : public TermMatchNode
    {
    public:
        Unigram(char const * text, char const * suffix, Classification classification);
        Unigram(IObjectParser& parser);

        //
        // IPersistableObject methods via TermMatchNode.
        //
        virtual void Format(IObjectFormatter& formatter) const override;

        //
        // TermMatchNode methods.
        //
        virtual NodeType GetType() const override;

        char const * GetText() const;
        char const * GetSuffix() const;
        Classification GetClassification() const;

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        char const * const m_text;
        Classification const m_classification;
        char const * const m_suffix;
    };


    class TermMatchNode::Fact : public TermMatchNode
    {
    public:
        Fact(FactHandle fact);
        Fact(IObjectParser& parser);

        FactHandle GetFact() const;

        //
        // TermMatchNode methods.
        //
        virtual NodeType GetType() const override;

        //
        // IPersistableObject methods via TermMatchNode.
        //
        virtual void Format(IObjectFormatter& formatter) const override;

    private:
        const FactHandle m_fact;
    };


    class TermMatchNode::Builder : NonCopyable
    {
    public:
        Builder(TermMatchNode const & parent,
                IAllocator& allocator);

        Builder(TermMatchNode::NodeType nodeType,
                IAllocator& allocator);

        void AddChild(TermMatchNode const * child);

        TermMatchNode const * Complete();

        static TermMatchNode const *
        CreatePhraseNode(StringVector const & grams,
                         char const * suffix,
                         Classification classification,
                         IAllocator& allocator);

        static TermMatchNode const *
        CreateUnigramNode(char const * text,
                          char const * suffix,
                          Classification classification,
                          IAllocator& allocator);

        static TermMatchNode const *
        CreateFactNode(FactHandle fact,
                       IAllocator& allocator);

    private:
        IAllocator& m_allocator;
        TermMatchNode::NodeType m_targetType;
        TermMatchNode const * m_firstChild;
        TermMatchNode const * m_node;
    };
}
