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

#include <stddef.h>                                 // For nullptr.

#include "BitFunnel/Allocators/IAllocator.h"        // Used by template definitions.
#include "BitFunnel/Term.h"                         // Used by template definitions.
#include "BitFunnel/Utilities/IObjectFormatter.h"   // Used by template definitions.
#include "BitFunnel/Utilities/IObjectParser.h"      // Used by template definitions.


namespace BitFunnel
{
    class IObjectFormatter;

    template <class NODE>
    NODE& ParseNode(IObjectParser& parser)
    {
        void* buffer = parser.GetAllocator().Allocate(sizeof(NODE));
        return *(new (buffer) NODE(parser));
    }


    template <class NODE>
    NODE const & ParseNodeField(IObjectParser& parser, char const *name)
    {
        parser.OpenObjectField(name);
        return dynamic_cast<NODE const &>(NODE::Parse(parser));
    }


    template <class NODE>
    NODE const * ParseNullableNodeField(IObjectParser& parser, char const *name)
    {
        parser.OpenObjectField(name);
        return dynamic_cast<NODE const*>(NODE::ParseNullable(parser));
    }


    template <typename T>
    T ParseField(IObjectParser& parser);


    template <typename T>
    T ParseObjectField(IObjectParser& parser, char const *name)
    {
        parser.OpenObjectField(name);
        return ParseField<T>(parser);
    }


    template <typename T>
    T ParsePrimitiveItem(IObjectParser& parser)
    {
        parser.OpenPrimitiveItem();
        return ParseField<T>(parser);
    }


    //*************************************************************************
    //
    // ParseField() template specializations.
    //
    //*************************************************************************

    template <>
    inline int ParseField<int>(IObjectParser& parser)
    {
        return parser.ParseInt();
    }


    template <>
    inline unsigned ParseField<unsigned>(IObjectParser& parser)
    {
        return parser.ParseUInt();
    }


    template <>
    inline uint64_t ParseField<uint64_t>(IObjectParser& parser)
    {
        return parser.ParseUInt64();
    }


    template <>
    inline double ParseField<double>(IObjectParser& parser)
    {
        return parser.ParseDouble();
    }


    template <>
    inline char const * ParseField<char const *>(IObjectParser& parser)
    {
        return parser.ParseStringLiteral();
    }


    //*************************************************************************
    //
    // BinaryTree formatting and parsing.
    //
    //*************************************************************************
    template <class T>
    void FormatList(T const & node, IObjectFormatter& formatter)
    {
        T const * left = dynamic_cast<T const *>(&node.GetLeft());
        if (left != nullptr)
        {
            FormatList<T>(*left, formatter);
        }
        else
        {
            formatter.OpenListItem();
            node.GetLeft().Format(formatter);
        }

        T const * right = dynamic_cast<T const *>(&node.GetRight());
        if (right != nullptr)
        {
            FormatList<T>(*right, formatter);
        }
        else
        {
            formatter.OpenListItem();
            node.GetRight().Format(formatter);
        }
    }


    template <class BASE, class DERIVED>
    BASE const & ParseList(IObjectParser& parser)
    {
        BASE const & head = BASE::Parse(parser);

        if (parser.OpenListItem())
        {
            BASE const & tail = ParseList<BASE, DERIVED>(parser);
            return *new (parser.GetAllocator().Allocate(sizeof(DERIVED)))
                        DERIVED(head, tail);
        }
        else
        {
            return head;
        }
    }
}
