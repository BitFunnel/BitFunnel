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
    ////template <int RDEST, int ROFFSET>
    //static void AndAccumulator(FunctionBuffer &  code,
    //                           Register<8u, false> rDest,
    //                           Register<8u, false> rOffset,
    //                           //X64Register<RDEST, 3> rDest,
    //                           //X64Register<ROFFSET, 3> rOffset,
    //                           unsigned rRow);


    void AndSIB(FunctionBuffer &  code,
                Register<8u, false> /*dest*/,
                Register<8u, false> /*index*/,
                Register<8u, false> /*base*/)
    {
        // TODO: Implement.
        code.Emit8(0xcc);
    }


    void MovSIB(FunctionBuffer &  code,
                Register<8u, false> /*dest*/,
                Register<8u, false> /*index*/,
                Register<8u, false> /*base*/)
    {
        // TODO: Implement.
        code.Emit8(0xcc);
    }


    void EmitCall(FunctionBuffer &  code,
                  NativeJIT::Label label)
    {
        // TODO: Implement.
        code.Emit8(0xcc);
        code.Jmp(label);
        code.Emit8(0xcc);
    }


    void EmitNot(FunctionBuffer &  code,
                 Register<8u, false> /*dest*/)
    {
        // TODO: Implement.
        code.Emit8(0xcc);
    }



    ////    template <int RDEST, int ROFFSET>
    //static void LoadAccumulator(FunctionBuffer &  code,
    //                            Register<8u, false> rDest,
    //                            Register<8u, false> rOffset, 
    //                            //X64Register<RDEST, 3> rDest,
    //                            //X64Register<ROFFSET, 3> rOffset,
    //                            unsigned rRow);


    // RAX: Scratch register
    // RBX: Accumulator
    // RCX: Loop counter at inner loop root, offset elsewhere in traversal
    //      Encoded as (&SLICE_POINTER >> 3) + offset.
    // RDX: Current slice pointer.
    // RSI: Pointer to array of all row offsets pointers.
    // RDI: &SLICE_POINTER >> 3
    // R8-R15: Register row pointers.

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
            if (!inverted)
            {
                if (m_registers.IsRegister(id))
                {
                    // Case 1: rankDelta > 0 && !inverted && IsRegister
                    // Calculate the offset into RAX.
                    m_code.Emit<OpCode::Mov>(rax, rcx);
                    m_code.Emit<OpCode::Sub>(rax, rdi); 
                    //m_code.MOV(RAX, RCX);
                    //m_code.SUB(RAX, RDI);

                    // Adjust the offset for the rank.
                    m_code.EmitImmediate<OpCode::Shr>(rax, static_cast<uint8_t>(rankDelta));
                    //m_code.SHR(RAX, rankDelta);

                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::Add>(rax, Register<8u, false>(reg));
                    AndSIB(m_code, rbx, rax, rdx);
                    //AndAccumulator(m_code, rbx, rax, reg);
                    //AndAccumulator(m_code, RBX, RAX, reg);
                }
                else
                {
                    // Case 2: rankDelta > 0 && !inverted && !IsRegister
                    // Save the original value in RCX. Calculate the offset which
                    // is stored in RCX and adjust it for the rank.
                    m_code.Emit<OpCode::Push>(rcx);
                    m_code.Emit<OpCode::Sub>(rcx, rdi);
                    m_code.EmitImmediate<OpCode::Shr>(rcx, rankDelta);
                    //m_code.PUSH(RCX);
                    //m_code.SUB(RCX, RDI);
                    //m_code.SHR(RCX, rankDelta);

                    // Get the offset for the row (stored in RAX) and combine with offsets within
                    // the row (stored in RCX) to form the overall offset with respect to the
                    // slice buffer pointer(stored in RDX).
                    // And the accumulator(RBX) with the quadword in the specified offset in the row.
                    m_code.Emit<OpCode::Mov>(rax, rsi, id * 8);
                    m_code.Emit<OpCode::Add>(rcx, rax);
                    AndSIB(m_code, rbx, rcx, rdx);
                    //m_code.MOV(RAX, QWORDPTR[RSI + static_cast<int>(id * 8)]);
                    //m_code.ADD(RCX, RAX);
                    //m_code.AND(RBX, QWORDPTR[SCALE8(RCX) + RDX]);

                    // POP doesn't affect flags.
                    m_code.Emit<OpCode::Pop>(rcx);
                    //m_code.POP(RCX);
                }
            }
            else
            {
                if (m_registers.IsRegister(id))
                {
                    // Case 3: rankDelta > 0 && inverted && IsRegister
                    // Calculate the offset into RAX.
                    m_code.Emit<OpCode::Mov>(rax, rcx);
                    m_code.Emit<OpCode::Sub>(rax, rdi);
                    //m_code.MOV(RAX, RCX);
                    //m_code.SUB(RAX, RDI);

                    // Adjust the offset for rank.
                    m_code.EmitImmediate<OpCode::Shr>(rax, rankDelta);
                    //m_code.SHR(RAX, rankDelta);

                    unsigned reg = m_registers.GetRegister(id);

                    m_code.Emit<OpCode::Add>(rax, Register<8u, false>(reg));
                    MovSIB(m_code, rax, rax, rdx);

                    //LoadAccumulator(m_code, rax, rax, reg);
                    //LoadAccumulator(m_code, RAX, RAX, reg);
                }
                else
                {
                    // Case 4: rankDelta > 0 && inverted && !IsRegister
                    // Save the original value in RCX. Calculate the offset which
                    // is stored in RCX and adjust it for the rank.
                    m_code.Emit<OpCode::Push>(rcx);
                    m_code.Emit<OpCode::Sub>(rcx, rdi);
                    m_code.EmitImmediate<OpCode::Shr>(rcx, rankDelta);
                    //m_code.PUSH(RCX);
                    //m_code.SUB(RCX, RDI);
                    //m_code.SHR(RCX, rankDelta);

                    // Get the offset for the row (stored in RAX) and combine with offsets within
                    // the row (stored in RCX) to form the overall offset with respect to the
                    // slice buffer pointer(stored in RDX).
                    // And the accumulator(RBX) with the quadword in the specified offset in the row.
                    m_code.Emit<OpCode::Mov>(rax, rsi, id * 8);
                    m_code.Emit<OpCode::Add>(rcx, rax);
                    MovSIB(m_code, rax, rcx, rdx);
                    //m_code.MOV(RAX, QWORDPTR[RSI + static_cast<int>(id * 8)]);
                    //m_code.ADD(RCX, RAX);
                    //m_code.MOV(RAX, QWORDPTR[SCALE8(RCX) + RDX]);

                    // POP doesn't affect flags.
                    m_code.Emit<OpCode::Pop>(rcx);
                    //m_code.POP(RCX);
                }

                // Acount for inverted row and And the data with accumulator(RBX).
                EmitNot(m_code, rax);
                m_code.Emit<OpCode::And>(rbx, rax);
                //m_code.NOT(RAX);
                //m_code.AND(RBX, RAX);
            }
        }
        else
        {
            if (!inverted)
            {
                // Calculate the offset into RAX.
                m_code.Emit<OpCode::Mov>(rax, rcx);
                m_code.Emit<OpCode::Sub>(rax, rdi);
                //m_code.MOV(RAX, RCX);
                //m_code.SUB(RAX, RDI);

                if (m_registers.IsRegister(id))
                {
                    // Case 5: rankDelta == 0 && !inverted && IsRegister                                 
                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::Add>(rax, Register<8u, false>(reg));
                    AndSIB(m_code, rbx, rax, rdx);
                    //AndAccumulator(m_code, rbx, rax, reg);
                    //AndAccumulator(m_code, RBX, RAX, reg);
                }
                else
                {
                    // Case 6: rankDelta == 0 && !inverted && !IsRegister
                    // Combine with offsets within the row (stored in RCX) with the row offset.
                    // And the accumulator(RBX) with the quadword in the specified offset in the row.
                    m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                    AndSIB(m_code, rbx, rax, rdx);
                    //m_code.ADD(RAX, QWORDPTR[RSI + static_cast<int>(id * 8)]);
                    //m_code.AND(RBX, QWORDPTR[SCALE8(RAX) + RDX]);
                }
            }
            else
            {
                // Row is inverted.
                m_code.Emit<OpCode::Mov>(rax, rcx);
                m_code.Emit<OpCode::Sub>(rax, rdi);
                //m_code.MOV(RAX, RCX);
                //m_code.SUB(RAX, RDI);

                if (m_registers.IsRegister(id))
                {
                    // Case 7: rankDelta == 0 && inverted && IsRegister
                    unsigned reg = m_registers.GetRegister(id);
                    m_code.Emit<OpCode::Add>(rax, Register<8u, false>(reg));
                    MovSIB(m_code, rax, rax, rdx);
                    //LoadAccumulator(m_code, rax, rax, reg);
                    //LoadAccumulator(m_code, RAX, RAX, reg);
                }
                else
                {
                    // Case 8: rankDelta == 0 && inverted && !IsRegister
                    // Combine with offsets within the row (stored in RCX) with the row offset.
                    // And the accumulator(RBX) with the quadword in the specified offset in the row.
                    m_code.Emit<OpCode::Mov>(rax, rsi, id * 8);
                    MovSIB(m_code, rax, rax, rdx);
                    //m_code.ADD(RAX, QWORDPTR[RSI + static_cast<int>(id * 8)]);
                    //m_code.MOV(RAX, QWORDPTR[SCALE8(RAX) + RDX]);
                }
                // Acount for inverted row and And the data with accumulator(RBX).
                EmitNot(m_code, rax);
                m_code.Emit<OpCode::And>(rbx, rax);
                //m_code.NOT(RAX);
                //m_code.AND(RBX, RAX);
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
            if (m_registers.IsRegister(id))
            {
                // Case 1: rankDelta > 0, IsRegister
                // Calculate the offset into RAX.
                m_code.Emit<OpCode::Mov>(rax, rcx);
                m_code.Emit<OpCode::Sub>(rax, rdi);
                //m_code.MOV(RAX, RCX);
                //m_code.SUB(RAX, RDI);

                // Adjust the offset for rank.
                m_code.EmitImmediate<OpCode::Shr>(rax, static_cast<uint8_t>(rankDelta));
                //m_code.SHR(RAX, rankDelta);

                unsigned reg = m_registers.GetRegister(id);
                m_code.Emit<OpCode::Add>(rax, Register<8u, false>(reg));
                MovSIB(m_code, rbx, rax, rdx);
                //LoadAccumulator(m_code, rbx, rax, reg);
                //LoadAccumulator(m_code, RBX, RAX, reg);
            }
            else
            {
                // Case 2: rankDelta > 0, !IsRegister
                // Save the original value in RCX. Calculate the offset which
                // is stored in RCX and adjust it for the rank.
                m_code.Emit<OpCode::Push>(rcx);
                m_code.Emit<OpCode::Sub>(rcx, rdi);
                m_code.EmitImmediate<OpCode::Shr>(rcx, rankDelta);
                //m_code.PUSH(RCX);
                //m_code.SUB(RCX, RDI);
                //m_code.SHR(RCX, rankDelta);

                // Get the offset for the row (stored in RAX) and combine with offsets within
                // the row (stored in RCX) to form the overall offset with respect to the
                // slice buffer pointer(stored in RDX).
                m_code.Emit<OpCode::Mov>(rax, rsi, id * 8);
                m_code.Emit<OpCode::Add>(rcx, rax);
                //m_code.MOV(RAX, QWORDPTR[RSI + static_cast<int>(id * 8)]);
                //m_code.ADD(RCX, RAX);

                // Load the accumulator(RBX) with the quadword in the specified offset in the row.
                MovSIB(m_code, rbx, rcx, rdx);
                m_code.Emit<OpCode::Pop>(rcx);
                //m_code.MOV(RBX, QWORDPTR[SCALE8(RCX) + RDX]);
                //m_code.POP(RCX);
            }
        }
        else
        {
            // Calculate the offset into RAX.
            m_code.Emit<OpCode::Mov>(rax, rcx);
            m_code.Emit<OpCode::Sub>(rax, rdi);
            //m_code.MOV(RAX, RCX);
            //m_code.SUB(RAX, RDI);

            if (m_registers.IsRegister(id))
            {
                // Case 3: rankDelta == 0, IsRegister
                unsigned reg = m_registers.GetRegister(id);
                m_code.Emit<OpCode::Add>(rax, Register<8u, false>(reg));
                MovSIB(m_code, rbx, rax, rdx);
                //LoadAccumulator(m_code, rbx, rax, reg);
                //LoadAccumulator(m_code, RBX, RAX, reg);
            }
            else
            {
                // Case 4: rankDelta == 0, !IsRegister
                // Combine with offsets within the row (stored in RAX) with the row offset
                // to form the overall offset with respect to the slice buffer pointer(stored in RDX).
                m_code.Emit<OpCode::Add>(rax, rsi, id * 8);
                //m_code.ADD(RAX, QWORDPTR[RSI + static_cast<int>(id * 8)]);

                // Load the accumulator(RBX) with the quadword in the specified offset in the row.
                MovSIB(m_code, rbx, rax, rdx);
                //m_code.MOV(RBX, QWORDPTR[SCALE8(RAX) + RDX]);
            }
        }

        if (inverted)
        {
            // NOTE that the X64 not opcode does not set the zero flag.
            // Must fall through to OR(RBX, RBX) to set flag appropriately.
            EmitNot(m_code, rax);
            //m_code.NOT(RBX);
        }

        // Make sure flags are set
        m_code.Emit<OpCode::Or>(rbx, rbx);
        //m_code.OR(RBX, RBX);
    }


    void MachineCodeGenerator::LeftShiftOffset(size_t shift)
    {
        // Decode the offset into RCX, adjust for the shift and then encode it back.
        m_code.Emit<OpCode::Sub>(rcx, rdi);
        m_code.EmitImmediate<OpCode::Shl>(rcx, static_cast<uint8_t>(shift));
        m_code.Emit<OpCode::Add>(rcx, rdi);
        //m_code.SUB(RCX, RDI);
        //m_code.SHL(RCX, shift);
        //m_code.ADD(RCX, RDI);
    }


    void MachineCodeGenerator::RightShiftOffset(size_t shift)
    {
        // Decode the offset into RCX, adjust for the shift and then encode it back.
        m_code.Emit<OpCode::Sub>(rcx, rdi);
        m_code.EmitImmediate<OpCode::Shr>(rcx, static_cast<uint8_t>(shift));
        m_code.Emit<OpCode::Add>(rcx, rdi);
        //m_code.SUB(RCX, RDI);
        //m_code.SHR(RCX, shift);
        //m_code.ADD(RCX, RDI);
    }


    void MachineCodeGenerator::IncrementOffset()
    {
        m_code.EmitImmediate<OpCode::Add>(rcx, 1);
        //m_code.INC(RCX);
    }



    void MachineCodeGenerator::Push()
    {
        m_code.Emit<OpCode::Push>(rbx);
        //m_code.PUSH(RBX);
        ++m_pushCount;
    }


    void MachineCodeGenerator::Pop()
    {
        m_code.Emit<OpCode::Pop>(rbx);
        //m_code.POP(RBX);
        --m_pushCount;
    }


    //
    // Stack machine primitives
    //
    void MachineCodeGenerator::AndStack()
    {
        m_code.Emit<OpCode::Pop>(rax);
        m_code.Emit<OpCode::And>(rbx, rax);
        //m_code.POP(RAX);
        //m_code.AND(RBX, RAX);
        --m_pushCount;
    }


    void MachineCodeGenerator::Constant(int value)
    {
        m_code.EmitImmediate<OpCode::Mov>(rbx, value);
        //m_code.MOV(RBX, value);
    }


    void MachineCodeGenerator::Not()
    {
        // TODO: NOT does not set the Z flag. Check if this code should OR(RBX, RBX).
        EmitNot(m_code, rbx);
        //m_code.NOT(RBX);
    }


    void MachineCodeGenerator::OrStack()
    {
        m_code.Emit<OpCode::Pop>(rax);
        m_code.Emit<OpCode::Or>(rbx, rax);
        //m_code.POP(RAX);
        //m_code.OR(RBX, RAX);
        --m_pushCount;
    }


    void MachineCodeGenerator::UpdateFlags()
    {
        m_code.Emit<OpCode::Or>(rbx, rbx);
        //m_code.OR(RBX, RBX);
    }


    void MachineCodeGenerator::Report()
    {
        // TODO: Implement.
        m_code.Emit8(0xcc);
    }
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
        EmitCall(m_code, NativeJIT::Label(label));
        //m_code.CALL(static_cast<X64::Label>(label));
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


    //// Helper function generates code for
    ////   Add(offset, rowOffset),
    ////   And(RDEST, QWORDPTR[SCALE8(offset) + slicePtr])
    ////template <int RDEST, int ROFFSET>
    //static void AndAccumulator(FunctionBuffer &  code,
    //                           Register<8u, false> rDest,
    //                           Register<8u, false> rOffset,
    //                           //X64Register<RDEST, 3> rDest,
    //                           //X64Register<ROFFSET, 3> rOffset,
    //                           unsigned rRow)
    //{
    //    CHECK_LE(rRow, 15u)
    //        << "Row register cannot exceed 15.";
    //    CHECK_GE(rRow, 8u)
    //        << "Row register cannot be less than 8.";

    //    code.Emit<OpCode::Add>(rOffset, Register<8u, false>(rRow));
    //    AndSIB(code, rDest, rOffset, rdx);

    //    //// Each case adjusts the offset for the corresponding row, 
    //    //// dereferences the slice data at the calculated offset and 
    //    //// ANDs the accumulator with the dereferenced value.
    //    //switch (rRow)
    //    //{
    //    //case 8:
    //    //    code.ADD(rOffset, R8);
    //    //    break;
    //    //case 9:
    //    //    code.ADD(rOffset, R9);
    //    //    break;
    //    //case 10:
    //    //    code.ADD(rOffset, R10);
    //    //    break;
    //    //case 11:
    //    //    code.ADD(rOffset, R11);
    //    //    break;
    //    //case 12:
    //    //    code.ADD(rOffset, R12);
    //    //    break;
    //    //case 13:
    //    //    code.ADD(rOffset, R13);
    //    //    break;
    //    //case 14:
    //    //    code.ADD(rOffset, R14);
    //    //    break;
    //    //case 15:
    //    //    code.ADD(rOffset, R15);
    //    //    break;
    //    //default:
    //    //    LogAbortB("Invalid row register.");
    //    //    break;
    //    //};

    //    //code.AND(rDest, QWORDPTR[SCALE8(rOffset) + RDX]);
    //}


    //// Helper function generates code for
    ////   ADD(offset, rowOffset)
    ////   MOV(RDEST, QWORDPTR[SCALE8(offset) + slicePtr])
    //// template <int RDEST, int ROFFSET>
    //static void LoadAccumulator(FunctionBuffer &  code,
    //                            Register<8u, false> rDest,
    //                            Register<8u, false> rOffset,
    //                            //X64Register<RDEST, 3> rDest,
    //                            //X64Register<ROFFSET, 3> rOffset,
    //                            unsigned rRow)
    //{
    //    CHECK_LE(rRow, 15u)
    //        << "Row register cannot exceed 15.";
    //    CHECK_GE(rRow, 8u)
    //        << "Row register cannot be less than 8.";

    //    code.Emit<OpCode::Add>(rOffset, Register<8u, false>(rRow));
    //    MovSIB(code, rDest, rOffset, rdx);

    //    //// Each case adjusts the offset for the corresponding row, 
    //    //// dereferences the slice data at the calculated offset and 
    //    //// loads the accumulator with the dereferenced value.
    //    //switch (rRow)
    //    //{
    //    //case 8:
    //    //    code.ADD(rOffset, R8);
    //    //    break;
    //    //case 9:
    //    //    code.ADD(rOffset, R9);
    //    //    break;
    //    //case 10:
    //    //    code.ADD(rOffset, R10);
    //    //    break;
    //    //case 11:
    //    //    code.ADD(rOffset, R11);
    //    //    break;
    //    //case 12:
    //    //    code.ADD(rOffset, R12);
    //    //    break;
    //    //case 13:
    //    //    code.ADD(rOffset, R13);
    //    //    break;
    //    //case 14:
    //    //    code.ADD(rOffset, R14);
    //    //    break;
    //    //case 15:
    //    //    code.ADD(rOffset, R15);
    //    //    break;
    //    //default:
    //    //    LogAbortB("Invalid row register.");
    //    //    break;
    //    //};

    //    //code.MOV(rDest, QWORDPTR[SCALE8(rOffset) + RDX]);
    //}
}
