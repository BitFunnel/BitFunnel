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


namespace BitFunnel
{
    //*************************************************************************
    //
    // ICodeGenerator is an abstract base class or interface for classes that
    // translate BitFunnel matching and scoring primitives into other forms
    // such as X64 machine code or a textual representation for unit tests.
    //
    //*************************************************************************
    class ICodeGenerator
    {
    public:
        typedef unsigned Label;

        virtual ~ICodeGenerator() {}

        // RankDown compiler primitives
        virtual void AndRow(size_t id, bool inverted, size_t rankDelta) = 0;
        virtual void LoadRow(size_t id, bool inverted, size_t rankDelta) = 0;

        virtual void LeftShiftOffset(size_t shift) = 0;
        virtual void RightShiftOffset(size_t shift) = 0;
        virtual void IncrementOffset() = 0;

        virtual void Push() = 0;
        virtual void Pop() = 0;

        // Stack machine primitives
        virtual void AndStack() = 0;
        virtual void Constant(int value) = 0;
        virtual void Not() = 0;
        virtual void OrStack() = 0;
        virtual void UpdateFlags() = 0;

        virtual void Report() = 0;

        // Control flow primitives.
        virtual Label AllocateLabel() = 0;
        virtual void PlaceLabel(Label label) = 0;
        virtual void Call(Label label) = 0;
        virtual void Jmp(Label label) = 0;
        virtual void Jnz(Label label) = 0;
        virtual void Jz(Label label) = 0;
        virtual void Return() = 0;
    };
}
