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

#include <istream>
#include <stack>

#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/Utilities/IObjectParser.h"


namespace BitFunnel
{
    class INode;
    class Term;

    class TextObjectParser : public IObjectParser, NonCopyable
    {
    public:
        typedef int (*TypenameConverter)(const char* name);

        TextObjectParser(std::istream& input,
                         IAllocator& allocator,
                         TypenameConverter typenameConverter);

        IAllocator& GetAllocator() const;

        int ReadTypeTag();

        void OpenObject();
        void OpenObjectField(const char* name);
        void CloseObject();

        void OpenList();
        bool OpenListItem();
        void CloseList();

        void OpenPrimitive(const char* name);
        bool OpenPrimitiveItem();
        void ClosePrimitive();

        bool ParseBool();
        int ParseInt();
        unsigned ParseUInt();
        uint64_t ParseUInt64();
        double ParseDouble();
        char const * ParseStringLiteral();
        void ParseToken(std::string& token);

    private:
        void SkipWhite();
        void Expect(char c);
        void Expect(const char* text);

        IAllocator& m_allocator;
        TypenameConverter m_typenameConverter;

        std::istream& m_input;

        std::stack<unsigned> m_objectFields;
        std::stack<unsigned> m_listItems;
        unsigned m_primitiveItems;
    };
}
