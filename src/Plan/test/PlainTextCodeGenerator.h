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

#include <ostream>                  // std::ostream parameter.

#include "BitFunnel/NonCopyable.h"  // Inherits from NonCopyable.
#include "ICodeGenerator.h"         // Inherits from ICodeGenerator.


namespace BitFunnel
{
    class PlainTextCodeGenerator : public ICodeGenerator, NonCopyable
    {
    public:
        PlainTextCodeGenerator(std::ostream& output);

        void AndRow(size_t id, bool inverted, size_t rankDelta);
        void LoadRow(size_t id, bool inverted, size_t rankDelta);

        void LeftShiftOffset(size_t shift);
        void RightShiftOffset(size_t shift);
        void IncrementOffset();

        void Push();
        void Pop();

        void AddStack();
        void AndStack();
        void Constant(int value);
        void MaxStack();
        void Not();
        void OrStack();
        void UpdateFlags();

        void Report();

        Label AllocateLabel();
        void PlaceLabel(Label label);
        void Call(Label label);
        void Jmp(Label label);
        void Jnz(Label label);
        void Jz(Label label);
        void Return();

    private:
        void EmitZeroArg(char const* name);
        void EmitSignedArg(char const* name, int arg);
        void EmitUnsignedArg(char const* name, unsigned arg);
        void EmitSizeTArg(char const* name, size_t arg);
        void EmitRowArg(char const* name,
                        size_t id,
                        bool inverted,
                        size_t rankDelta);
        void Indent();

        std::ostream& m_output;
        Label m_label;
    };
}
