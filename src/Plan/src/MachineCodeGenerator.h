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

#include "BitFunnel/Plan/ICodeGenerator.h"     // Base class.
#include "BitFunnel/NonCopyable.h"        // Base class.


namespace NativeJIT
{
    class FunctionBuffer;
}

using namespace NativeJIT;


namespace BitFunnel
{
    class RegisterAllocator;


    //*************************************************************************
    //
    // MachineCodeGenerator is an ICodeGenerator used to translate CompileNode
    // trees into native X64 code.
    //
    // Methods in MachineCodeGenerator are called by CompileNode::Compile() and
    // by ScoreTreeCodeGenerator::CompileScoreTree().
    //
    //*************************************************************************
    class MachineCodeGenerator : public ICodeGenerator, NonCopyable
    {
    public:
        // Constructs a MachineCodeGenerator which generates X64 code using the
        // supplied X64FunctionGenerator. The registers parameter supplies a
        // RegisterAllocator that provides register assignments for some rows.
        MachineCodeGenerator(RegisterAllocator const & registers,
                             FunctionBuffer & code);

        //
        // ICodeGenerator methods
        //

        // RankDown compiler primitives
        void AndRow(unsigned id, bool inverted, unsigned rankDelta);
        void LoadRow(unsigned id, bool inverted, unsigned rankDelta);

        void LeftShiftOffset(unsigned shift);
        void RightShiftOffset(unsigned shift);
        void IncrementOffset();

        void Push();
        void Pop();

        // Stack machine primitives
        void AndStack();
        void Constant(int value);
        void Not();
        void OrStack();
        void UpdateFlags();

        void Report();

        // Constrol flow primitives.
        Label AllocateLabel();
        void PlaceLabel(Label label);
        void Call(Label label);
        void Jmp(Label label);
        void Jnz(Label label);
        void Jz(Label label);
        void Return();


        // Returns the number of the first row pointer register. Registers
        // should be assigned starting from 
        //    GetRegisterBase()
        // and continuing through to 
        //    GetRegisterBase() + GetRegsiterCount() -1
        // WARNING: MachineCodeGenerator assumes that registers are allocated
        // in this order. This is important when the number of registers
        // actually allocated is less than the number of registers available.
        static unsigned GetRegisterBase();

        // Returns the number of registers available for holding row pointers.
        // This value should be passed to the constructor of the register
        // allocator.
        static unsigned GetRegisterCount();

        // Returns the number of stack slots reserved for local variables and
        // parameter homes for calls to the static AddResultsHelper() and
        // FinishIterationHelper() methods.
        static unsigned GetSlotCount();

    protected:
        //
        // Constructor parameters
        //

        RegisterAllocator const & m_registers;

        FunctionBuffer & m_code;


        // Records the number of items pushed on the X64 stack since the
        // stack frame setup was completed. Required to satisfy X64 calling
        // convention rules that require RSP be 16-byte aligned when allocating
        // space for function call parameter homes. See Report() function for
        // more information.
        unsigned m_pushCount;

        // First available row pointer register is R8.
        static const unsigned c_registerBase = 8;

        // Row pointers stored in the eight registers R8..R15.
        static const unsigned c_registerCount = 8;

        // The number of stack slots reserved for local variables and
        // parameter homes for calls to the static AddResultsHelper() and
        // FinishIterationHelper() methods. Since we have no local variables,
        // the number of slots is dictated by the callback with the most
        // parameters, which is AddResultsHelper() with 3 parameters.
        static const unsigned c_slotCount = 3;
    };
}
