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

#include "LoggerInterfaces/Check.h"
#include "MachineCodeGenerator.h"
#include "NativeCodeGenerator.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/CodeGen/X64CodeGenerator.h"
#include "RegisterAllocator.h"

using namespace NativeJIT;


namespace BitFunnel
{
    // New register scheme:
    //
    // slice the pointer to the current slice buffer
    // iteration is the iteration count in the inner loop
    //
    // rax: scratch register
    // rbx: accumulator
    // rcx: slice + iteration
    // rdx: slice
    // rsi: pointer to array of row offsets
    // rdi: pointer to parameters data structure
    // r8-r15: row offset pointers


    MachineCodeGenerator::MachineCodeGenerator(RegisterAllocator const & registers,
                                               FunctionBuffer & code)
      : m_registers(registers),
        m_code(code),
        m_pushCount(0)
    {
    }


    //
    // ICodeGenerator methods
    //

    //
    // RankDown compiler primitives
    //
    void MachineCodeGenerator::AndRow(size_t row,
                                      bool inverted,
                                      size_t rd)
    {
#ifdef QUADWORDCOUNT
        m_code.Emit<OpCode::Inc, 8>(rdi, NativeCodeGenerator::m_quadwordCount);
#endif

        unsigned id = static_cast<unsigned>(row);
        const uint8_t rankDelta = static_cast<uint8_t>(rd);

        // TODO: Need to deal with out of order loads.
        if (rankDelta > 0)
        {
            // Store rank-adjusted offset in rax.
            m_code.Emit<OpCode::Mov>(rax, rcx);
            m_code.Emit<OpCode::Sub>(rax, rdx);
            m_code.EmitImmediate<OpCode::Shr>(rax, static_cast<uint8_t>(rankDelta + 3));
            m_code.EmitImmediate<OpCode::Shl>(rax, static_cast<uint8_t>(3));

            if (!inverted)
            {
                if (m_registers.IsRegister(id))
                {
                    // Case 1: rankDelta > 0 && !inverted && IsRegister
                    m_code.Emit<OpCode::Add>(rax, rdx);
                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::And>(rbx, rax, Register<8u, false>(reg), SIB::Scale1, 0);
                }
                else
                {
                    // Case 2: rankDelta > 0 && !inverted && !IsRegister
                    m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                    m_code.Emit<OpCode::And>(rbx, rax, rdx, SIB::Scale1, 0);
                }
            }
            else
            {
                if (m_registers.IsRegister(id))
                {
                    // Case 3: rankDelta > 0 && inverted && IsRegister
                    m_code.Emit<OpCode::Add>(rax, rdx);
                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::Mov>(rax, rax, Register<8u, false>(reg), SIB::Scale1, 0);
                }
                else
                {
                    // Case 4: rankDelta > 0 && inverted && !IsRegister
                    m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                    m_code.Emit<OpCode::Mov>(rax, rax, rdx, SIB::Scale1, 0);
                }

                // Combine inverted row with accumulator(RBX).
                m_code.Emit<OpCode::Not>(rax);
                m_code.Emit<OpCode::And>(rbx, rax);
            }
        }
        else
        {
            if (!inverted)
            {
                if (m_registers.IsRegister(id))
                {
                    // Case 5: rankDelta == 0 && !inverted && IsRegister
                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::And>(rbx, rcx, Register<8u, false>(reg), SIB::Scale1, 0);
                }
                else
                {
                    // Case 6: rankDelta == 0 && !inverted && !IsRegister
                    m_code.Emit<OpCode::Mov>(rax, rcx);
                    m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                    m_code.Emit<OpCode::And>(rbx, rax, 0);
                }
            }
            else
            {
                // Row is inverted.
                if (m_registers.IsRegister(id))
                {
                    // Case 7: rankDelta == 0 && inverted && IsRegister
                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::Mov>(rax, rcx, Register<8u, false>(reg), SIB::Scale1, 0);
                }
                else
                {
                    // Case 8: rankDelta == 0 && inverted && !IsRegister
                    m_code.Emit<OpCode::Mov>(rax, rcx);
                    m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                    m_code.Emit<OpCode::Mov>(rax, rax, 0);
                }

                // Combine inverted row with accumulator(RBX).
                m_code.Emit<OpCode::Not>(rax);
                m_code.Emit<OpCode::And>(rbx, rax);
            }
        }

        // No need to OR(RBX, RBX) because flags are already set by AND
        // operation.
    }


    void MachineCodeGenerator::LoadRow(size_t row,
                                       bool inverted,
                                       size_t rd)
    {
#ifdef QUADWORDCOUNT
        m_code.Emit<OpCode::Inc, 8>(rdi, NativeCodeGenerator::m_quadwordCount);
#endif

        unsigned id = static_cast<unsigned>(row);
        uint8_t rankDelta = static_cast<uint8_t>(rd);

        if (rankDelta > 0)
        {
            // Store rank-adjusted offset in rax.
            m_code.Emit<OpCode::Mov>(rax, rcx);
            m_code.Emit<OpCode::Sub>(rax, rdx);
            m_code.EmitImmediate<OpCode::Shr>(rax, static_cast<uint8_t>(rankDelta + 3));
            m_code.EmitImmediate<OpCode::Shl>(rax, static_cast<uint8_t>(3));

            if (m_registers.IsRegister(id))
            {
                // Case 1: rankDelta > 0, IsRegister
                m_code.Emit<OpCode::Add>(rax, rdx);
                unsigned reg = m_registers.GetRegister(id);
                m_code.Emit<OpCode::Mov>(rbx, rax, Register<8u, false>(reg), SIB::Scale1, 0);
            }
            else
            {
                // Case 2: rankDelta > 0, !IsRegister
                m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                m_code.Emit<OpCode::Mov>(rbx, rax, rdx, SIB::Scale1, 0);
            }
        }
        else
        {
            if (m_registers.IsRegister(id))
            {
                // Case 3: rankDelta == 0, IsRegister
                unsigned reg = m_registers.GetRegister(id);
                m_code.Emit<OpCode::Mov>(rbx, rcx, Register<8u, false>(reg), SIB::Scale1, 0);
            }
            else
            {
                // Case 4: rankDelta == 0, !IsRegister
                m_code.Emit<OpCode::Mov>(rax, rcx);
                m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                m_code.Emit<OpCode::Mov>(rbx, rax, 0);
            }
        }

        if (inverted)
        {
            // NOTE that the X64 not opcode does not set the zero flag.
            // Must fall through to OR(RBX, RBX) to set flag appropriately.
            m_code.Emit<OpCode::Not>(rbx);
        }

        // Make sure flags are set
        m_code.Emit<OpCode::Or>(rbx, rbx);
    }


    void MachineCodeGenerator::LeftShiftOffset(size_t shift)
    {
        // Decode the offset into RCX, adjust for the shift and then encode it back.
        m_code.Emit<OpCode::Sub>(rcx, rdx);
        m_code.EmitImmediate<OpCode::Shl>(rcx, static_cast<uint8_t>(shift));
        m_code.Emit<OpCode::Add>(rcx, rdx);
    }


    void MachineCodeGenerator::RightShiftOffset(size_t shift)
    {
        // Decode the offset into RCX, adjust for the shift and then encode it back.
        m_code.Emit<OpCode::Sub>(rcx, rdx);
        m_code.EmitImmediate<OpCode::Shr>(rcx, static_cast<uint8_t>(shift + 3));
        m_code.EmitImmediate<OpCode::Shl>(rcx, static_cast<uint8_t>(3));
        m_code.Emit<OpCode::Add>(rcx, rdx);
    }


    void MachineCodeGenerator::IncrementOffset()
    {
        m_code.EmitImmediate<OpCode::Add>(rcx, 8);
    }


    void MachineCodeGenerator::Push()
    {
        m_code.Emit<OpCode::Push>(rbx);
        ++m_pushCount;
    }


    void MachineCodeGenerator::Pop()
    {
        m_code.Emit<OpCode::Pop>(rbx);
        --m_pushCount;
    }


    //
    // Stack machine primitives
    //
    void MachineCodeGenerator::AndStack()
    {
        m_code.Emit<OpCode::Pop>(rax);
        m_code.Emit<OpCode::And>(rbx, rax);
        --m_pushCount;
    }


    void MachineCodeGenerator::Constant(int value)
    {
        m_code.EmitImmediate<OpCode::Mov>(rbx, value);
    }


    void MachineCodeGenerator::Not()
    {
        // TODO: NOT does not set the Z flag. Check if this code should OR(RBX, RBX).
        m_code.Emit<OpCode::Not>(rbx);
    }


    void MachineCodeGenerator::OrStack()
    {
        m_code.Emit<OpCode::Pop>(rax);
        m_code.Emit<OpCode::Or>(rbx, rax);
        --m_pushCount;
    }


    void MachineCodeGenerator::UpdateFlags()
    {
        m_code.Emit<OpCode::Or>(rbx, rbx);
    }


    void MachineCodeGenerator::Report()
    {
        // Free up a register.
        m_code.Emit<OpCode::Push>(rcx);

        // Compute iteration number in rax.
        m_code.Emit<OpCode::Sub>(rcx, rdx);
        m_code.EmitImmediate<OpCode::Shr>(rcx, static_cast<uint8_t>(3));
        m_code.Emit<OpCode::Sub>(rcx, rdi, NativeCodeGenerator::m_base);

        // Mark the quadword for this iteration.
        m_code.EmitImmediate<OpCode::Mov>(rax, 1);
        m_code.Emit<OpCode::Shl>(rax);
        m_code.Emit<OpCode::Or>(rdi, NativeCodeGenerator::m_dedupe, rax);

        // Or the accumulator into that quadword.
        m_code.Emit<OpCode::Or>(rdi,
                                rcx,
                                SIB::Scale8, 8 + NativeCodeGenerator::m_dedupe,
                                rbx);

        // Restore registers.
        m_code.Emit<OpCode::Pop>(rcx);
    }


    // Control flow primitives.
    ICodeGenerator::Label MachineCodeGenerator::AllocateLabel()
    {
        return m_code.AllocateLabel().GetId();
    }


    void MachineCodeGenerator::PlaceLabel(Label label)
    {
        m_code.PlaceLabel(NativeJIT::Label(label));
    }


    void MachineCodeGenerator::Call(Label label)
    {
        m_pushCount++;
        m_code.Call(NativeJIT::Label(label));
        m_pushCount--;
    }


    void MachineCodeGenerator::Jmp(Label label)
    {
        m_code.Jmp(NativeJIT::Label(label));
    }


    void MachineCodeGenerator::Jnz(Label label)
    {
        m_code.EmitConditionalJump<JccType::JNZ>(NativeJIT::Label(label));
    }


    void MachineCodeGenerator::Jz(Label label)
    {
        m_code.EmitConditionalJump<JccType::JZ>(NativeJIT::Label(label));
    }


    void MachineCodeGenerator::Return()
    {
        m_code.Emit<OpCode::Ret>();
    }


    unsigned MachineCodeGenerator::GetRegisterBase()
    {
        return c_registerBase;
    }


    unsigned MachineCodeGenerator::GetRegisterCount()
    {
        return c_registerCount;
    }


    unsigned MachineCodeGenerator::GetSlotCount()
    {
        return c_slotCount;
    }
}
