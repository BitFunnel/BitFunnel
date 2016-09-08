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

#include "BitFunnel/AbstractRow.h"
#include "LoggerInterfaces/Logging.h"
#include "NodeUtilities.h"


namespace BitFunnel
{
    template <>
    inline AbstractRow ParseField<AbstractRow>(IObjectParser& parser)
    {
        return AbstractRow(parser, false);
    }


    template <class T>
    static T const & ParseBinaryTree(IObjectParser& parser, const char* childrenFieldName)
    {
        parser.OpenObject();
        parser.OpenObjectField(childrenFieldName);

        parser.OpenList();

        LogAssertB(parser.OpenListItem(),
                   "binary nodes must have exactly two children; only found one?");
        T const & node = dynamic_cast<T const &>(ParseList<CompileNode, T>(parser));

        parser.CloseList();
        parser.CloseObject();

        return node;
    }
}
