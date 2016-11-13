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
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/CodeGen/X64CodeGenerator.h"
#include "RegisterAllocator.h"

using namespace NativeJIT;


namespace BitFunnel
{
    // RAX: Scratch register
    // RBX: Accumulator
    // RCX: Loop counter at inner loop root, offset elsewhere in traversal
    //      Encoded as (&SLICE_POINTER >> 3) + offset.
    // RDX: Current slice pointer.
    // RSI: Pointer to array of all row offsets pointers.
    // RDI: &SLICE_POINTER >> 3
    // R8-R15: Register row pointers.

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
    //
    // while (sliceCount > 0)
    // {
    //    
    // }


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
        unsigned id = static_cast<unsigned>(row);
        const uint8_t rankDelta = static_cast<uint8_t>(rd);

        // TODO: Need to deal with out of order loads.
        if (rankDelta > 0)
        {
            // Store rank-adjusted offset in rax.
            m_code.Emit<OpCode::Mov>(rax, rcx);
            m_code.Emit<OpCode::Sub>(rax, rdx);
            m_code.EmitImmediate<OpCode::Shr>(rax, static_cast<uint8_t>(rankDelta));

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
                    // Case 5: rankDelta == 0 && !inverted && IsRegister                              m_code.Emit<OpCode::Mov>(rax, rcx);
                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::And>(rbx, rcx, Register<8u, false>(reg), SIB::Scale1, 0);
                }
                else
                {
                    // Case 6: rankDelta == 0 && !inverted && !IsRegister
                    m_code.Emit<OpCode::Mov>(rax, rcx);
                    m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                    m_code.Emit<OpCode::And>(rbx, rax, rdx, SIB::Scale1, 0);
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
                    m_code.Emit<OpCode::Mov>(rax, rax, rdx, SIB::Scale1, 0);
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
        unsigned id = static_cast<unsigned>(row);
        uint8_t rankDelta = static_cast<uint8_t>(rd);

        if (rankDelta > 0)
        {
            // Store rank-adjusted offset in rax.
            m_code.Emit<OpCode::Mov>(rax, rcx);
            m_code.Emit<OpCode::Sub>(rax, rdx);
            m_code.EmitImmediate<OpCode::Shr>(rax, static_cast<uint8_t>(rankDelta));

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
                m_code.Emit<OpCode::Mov>(rbx, rax, rdx, SIB::Scale1, 0);
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
        m_code.EmitImmediate<OpCode::Shr>(rcx, static_cast<uint8_t>(shift));
        m_code.Emit<OpCode::Add>(rcx, rdx);
    }


    void MachineCodeGenerator::IncrementOffset()
    {
        m_code.Emit<OpCode::Inc>(rcx);
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
        // TODO: Implement.
        m_code.Emit8(0xcc);
    }


    //void MachineCodeGenerator::Report()
    //{
    //    // TODO: set matchFound.

    //}

#if 0
    void MachineCodeGenerator::Report()
    {
        // Save volatile registers
        m_code.PUSH(RCX);
        m_code.PUSH(RDX);
        m_code.PUSH(R8);
        m_code.PUSH(R9);
        m_code.PUSH(R10);
        m_code.PUSH(R11);
        m_pushCount += 6;

        // Record the fact that we found a match in this group.
        m_code.MOV(R10, 1);
        m_code.MOV(m_code.Local(0), R10);

        // Plan to allocate one slot for each of the 3 parameters to
        // ProcessResultsHelper.
        int parameterCount = 3;
        const int x64MinSlots = 4;
        int slots = (std::max)(parameterCount, x64MinSlots);

        // Ensure RSP is 16-byte aligned while allocating space for function call parameter homes.
        // If we've pushed an odd number of times in the combination of EmitPush()/EmitPop and 
        // saving 6 volatiles, add one more slot to ensure the stack will be 16-byte aligned.
        if ((m_pushCount + slots) % 2 != 0)
        {
            ++slots;                        // Add one more slot to align RSP to 16-byte boundary.
        }
        m_code.SUB(RSP, slots * 8);

        // Set up parameters to the function call.
        // Please refer to EmitRegisterInitialization() function
        // in MatchTreeCodeGenerator.cpp to understand the usage 
        // of m_code.ParameterHome(4) and m_code.ParameterHome(6).
        m_code.MOV(R8, RCX);                            // Third parameter is the iteration counter.
        m_code.SUB(R8, RDI);

        m_code.MOV(RDX, RBX);                           // Second parameter is accumulator value in RBX.
        m_code.MOV(RCX, m_code.ParameterHome(4));       // First parameter is the IResultsProcessor.

        m_code.MOV(RAX, m_code.ParameterHome(6));       // This is the address of AddResultsHelper passed as a parameter
        m_code.CALL(RAX);

        // Restore RSP
        m_code.ADD(RSP, slots * 8);

        // Restore volatiles
        m_code.POP(R11);
        m_code.POP(R10);
        m_code.POP(R9);
        m_code.POP(R8);
        m_code.POP(RDX);
        m_code.POP(RCX);
        m_pushCount -= 6;
    }
#endif


    // Constrol flow primitives.
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
