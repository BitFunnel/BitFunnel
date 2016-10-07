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

#include <ostream>
#include <stack>

#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/Utilities/IObjectFormatter.h"


namespace BitFunnel
{
    class Term;


    class TextObjectFormatter : public IObjectFormatter, NonCopyable
    {
    public:
        TextObjectFormatter(std::ostream& output);
        TextObjectFormatter(std::ostream& output, unsigned indentation);

        void OpenObject(const IPersistableObject& object);
        void OpenObjectField(char const * name);
        void CloseObject();

        void NullObject();

        void OpenList();
        void OpenListItem();
        void CloseList();

        void OpenPrimitive(char const * name);
        void OpenPrimitiveItem();
        void ClosePrimitive();

        void Format(bool value);
        void Format(int value);
        void Format(unsigned value);
        void Format(size_t value);
        void Format(double value);
        void Format(char const * value);
        void FormatStringLiteral(char const * value);

    private:
        void Indent();

        std::ostream& m_output;
        unsigned m_indentation;

        std::stack<unsigned> m_objectFields;
        std::stack<unsigned> m_listItems;
        unsigned m_primitiveItems;
    };
}
