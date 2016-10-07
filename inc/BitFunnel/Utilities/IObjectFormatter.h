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

#include <cstddef>

namespace BitFunnel
{
    class IPersistableObject;

    class IObjectFormatter
    {
    public:
        virtual ~IObjectFormatter() {}

        virtual void OpenObject(const IPersistableObject& object) = 0;
        virtual void OpenObjectField(const char* name) = 0;
        virtual void CloseObject() = 0;

        virtual void NullObject() = 0;

        virtual void OpenList() = 0;
        virtual void OpenListItem() = 0;
        virtual void CloseList() = 0;

        virtual void OpenPrimitive(const char* name) = 0;
        virtual void OpenPrimitiveItem() = 0;
        virtual void ClosePrimitive() = 0;

        virtual void Format(bool value) = 0;
        virtual void Format(int value) = 0;
        virtual void Format(unsigned value) = 0;
        virtual void Format(size_t value) = 0;
        virtual void Format(double value) = 0;
        virtual void Format(const char* value) = 0;
        virtual void FormatStringLiteral(const char* value) = 0;
    };
}
