#include "stdafx.h"

#include <algorithm>    // For std::min() or std::max()

#include "CompileNodes.h"
#include "MatchTreeCodeGenerator.h"
#include "RegisterAllocator.h"
#include "Rounding.h"
#include "X64/X64FunctionGenerator.h"


using namespace X64;


namespace BitFunnel
{
    MatchTreeCodeGenerator::MatchTreeCodeGenerator(RegisterAllocator const & registers,
                                                   X64::X64FunctionGenerator& code,
                                                   unsigned maxIterationsScannedBetweenTerminationChecks)
        : MachineCodeGenerator(registers, code),
          m_maxIterationsScannedBetweenTerminationChecks(maxIterationsScannedBetweenTerminationChecks),
          // Because we use the TEST instruction on the iteration count register and we
          // want a regular callback frequency, we need to use a bitmask with all bits
          // set from the LSB to position n.
          m_maxIterationsScannedBetweenTerminationChecksMask(
            static_cast<unsigned>(RoundUpPowerOf2(maxIterationsScannedBetweenTerminationChecks) - 1))
    {
    }


    void MatchTreeCodeGenerator::GenerateX64Code(CompileNode const & root)
    {
        // Always clear the X64FunctionGenerator before starting. This retains
        // the stack unwind information configured in the X64FunctionGenerator
        // constructor along with the stack frame construction code, while
        // setting the code insertion point immediately after the stack frame
        // construction code.
        m_code.Clear();

        EmitRegisterInitialization();
        EmitOuterLoop(root);
        EmitFunctionReturn();
    }

    void MatchTreeCodeGenerator::EmitRegisterInitialization()
    {
        // Arguments
        //   0: RCX: void const ** array of slice pointers.
        //   1: RDX: unsigned number of slices.
        //   2: R8: unsigned __int64 const * array of row offsets in a single slice.
        //   3: R9: unsigned number of iterations in a single slices (the number of quadwords in a single slice).
        //   4: IResultsProcessor &
        //   5: long long address of FinishIterationHelper
        //   6: long long address of AddResultsHelper

        // Save various registers in its parameter home.
        m_code.MOV(m_code.ParameterHome(0), RCX);
        m_code.MOV(m_code.ParameterHome(1), RDX);
        m_code.MOV(m_code.ParameterHome(2), R8);
        m_code.MOV(m_code.ParameterHome(3), R9);

        // RSI has pointer to row offsets.
        m_code.MOV(RSI, R8);

        // DESIGN NOTE: Use parameter home 0 to store the slice pointers. Use parameter home 1 to store
        // the slice count. In the outer loop, slice count is decremented each time while the slice pointer
        // is incremented by 8 each time. When the slice count (the value stored in parameter home 1) is 
        // zero, the evaluation loop exits.

        // Load row offsets into R8..R8 + m_registers.GetRegistersAllocated()
        int n = m_registers.GetRegistersAllocated();
        switch (n)
        {
        case 8:
            m_code.MOV(R15, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(7)) * 8]);
            // Deliberately fall through to next case.
        case 7:
            m_code.MOV(R14, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(6)) * 8]);
            // Deliberately fall through to next case.
        case 6:
            m_code.MOV(R13, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(5)) * 8]);
            // Deliberately fall through to next case.
        case 5:
            m_code.MOV(R12, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(4)) * 8]);
            // Deliberately fall through to next case.
        case 4:
            m_code.MOV(R11, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(3)) * 8]);
            // Deliberately fall through to next case.
        case 3:
            m_code.MOV(R10, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(2)) * 8]);
            // Deliberately fall through to next case.
        case 2:
            m_code.MOV(R9, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(1)) * 8]);
            // Deliberately fall through to next case.
        case 1:
            m_code.MOV(R8, QWORDPTR[RSI + static_cast<int>(m_registers.GetRowIdFromRegister(0)) * 8]);
            // Deliberately fall through to next case.
        default:
            ;
        }
    }


    void MatchTreeCodeGenerator::EmitOuterLoop(CompileNode const & root)
    {
        Label topOfLoop = m_code.AllocateLabel();
        Label exitLoop = m_code.AllocateLabel();

        m_code.PlaceLabel(topOfLoop);

        // Check if the slice count (loop counter) reaches zero.
        m_code.MOV(RAX, m_code.ParameterHome(1));
        m_code.OR(RAX, RAX);
        m_code.JZ(exitLoop, X64::LONG);

        // RDI has the current slice pointer, expressed in the format of 
        // the number of quadwords from address 0 to the address of the 
        // current slice pointer(i.e. shifted right by 3).
        m_code.MOV(RDI, m_code.ParameterHome(0));
        m_code.MOV(RDI, QWORDPTR[RDI]);
        m_code.SHR(RDI, 3);

        EmitInnerLoop(root);

        // Decrement the slice count by 1.
        m_code.DEC(m_code.ParameterHome(1));

        // RAX contains the return values from EmitInnerLoop(), which 
        // are a flag to indicate whether the outer loop should early
        // terminate and a value to indicate the number of quadwords 
        // have not been scanned in the current slice represented by RDI.
        // The higher 32 bits of RAX contains the flag to indicates if
        // the outer loop should early terminate. The lower 32 bits
        // stores the number of quadwords have not been scanned in the
        // current slice.
        // Safe RAX to RDX temporarily.
        m_code.MOV(RDX, RAX);

        // Get the higher 32 bits of RAX.
        m_code.SHR(RAX, 32);

        // Check the early termination flag.
        m_code.XOR(RAX, 0);
        m_code.JNZ(exitLoop, X64::LONG);

        // Advance to the next slice.
        m_code.ADD(m_code.ParameterHome(0), 8ul);
        m_code.JMP(topOfLoop, X64::LONG);

        m_code.PlaceLabel(exitLoop);
        
        // Store the total number of quadwords have not been scanned by
        // the matcher to RAX. This will be returned to the c++ code which
        // calls this compiled code.
        // Clear RAX first. RDX has a copy of the original value in RAX.
        m_code.XOR(RAX, RAX);

        // Use number of left slices multiplies with the number of quadwords
        // in a slice, plus the number of left quadword in the last slice, get the
        // total number of unscanned quadwords.
        //
        // Compute the number of left quadwords in the unscanned slices.
        // Need to push RDX to save the current value in RDX because MUL will
        // put the higher bits in the multiplication to EDX.
        m_code.PUSH(RDX);
        m_code.MOV(RAX, m_code.ParameterHome(1));
        m_code.MUL(m_code.ParameterHome(3));
        m_code.POP(RDX);

        // Manipulate RDX to get the lower 32 bits, which is the number of unscanned
        // quadwords in the last scanned slice.
        m_code.SHL(RDX, 32UL);
        m_code.SHR(RDX, 32UL);

        // Get the total number of unscanned quadwords.
        m_code.ADD(RAX, RDX);
    }


    void MatchTreeCodeGenerator::EmitInnerLoop(CompileNode const & root)
    {
        Label topOfLoop = m_code.AllocateLabel();
        Label exitLoop = m_code.AllocateLabel();
        Label earlyTerminate = m_code.AllocateLabel();
        Label bottomOfLoop = m_code.AllocateLabel();
        Label finishIteration = m_code.AllocateLabel();

        // Initialize loop counter and limit.
        // Loop counter starts at the current slice pointer, expressed in the format of 
        // the number of quadwords from address 0 to the address of the current slice 
        // pointer(i.e. shifted right by 3).
        // Loop limit is equal to the counter plus the number of quadwords in a slice.
        m_code.MOV(RCX, RDI);
        m_code.MOV(RDX, m_code.ParameterHome(3));
        m_code.ADD(RDX, RCX);

        // Reuse ParameterHome(2) to save the iteration limit in the inner loop.
        m_code.MOV(m_code.ParameterHome(2), RDX);

        // Use RDX to store the slice pointer for the current slice.
        m_code.MOV(RDX, m_code.ParameterHome(0));
        m_code.MOV(RDX, QWORDPTR[RDX]);

        // Top of loop
        m_code.PlaceLabel(topOfLoop);
        m_code.CMP(RCX, m_code.ParameterHome(2));                  // Exit when loop counter RCX == iterations
        m_code.JE(exitLoop, X64::LONG);

        //
        // Body of loop
        //

        // TODO: Handle case where there are no rows.

        // Clear Local(0) to indicate that no matches have been found on this iteration.
        m_code.MOV(RAX, 0ULL);
        m_code.MOV(m_code.Local(0), RAX);

        root.Compile(*this);

        // If at least one match was found in this iteration, call FinishIteration.
        m_code.MOV(RAX, m_code.Local(0));
        m_code.OR(RAX, RAX);
        
        if (m_maxIterationsScannedBetweenTerminationChecks == 0)
        {
            // If there were no matches, skip reporting the end of the iteration
            // and go to the bottom of the loop.
            m_code.JZ(bottomOfLoop, X64::LONG);
        }
        else
        {
            // If there were matches, directly perform the iteration end callback.
            m_code.JNZ(finishIteration, X64::LONG);

            // Load the slice end iteration pointer.
            m_code.MOV(RAX, m_code.ParameterHome(2));

            // Subtract the current iteration pointer to know how many iterations
            // are left to execute.
            m_code.SUB(RAX, RCX);

            // Test whether the remaining number of iterations is a multiple of
            // the number of iterations between termination checks.
            m_code.TEST(RAX, m_maxIterationsScannedBetweenTerminationChecksMask);

            // If it is not a multiple, go to the next iteration.
            m_code.JNZ(bottomOfLoop, X64::LONG);
        }

        m_code.PlaceLabel(finishIteration);

        EmitFinishIteration();

        // The return value of the FinishIterationHelper function
        // should be stored in EAX. Check the value of EAX.
        // If the EAX value is 0, it means the query execution should 
        // continue, otherwise, the execution should terminate.
        //
        // The return value from EmitFinishIteration() is a boolean
        // and in retail build, only the lowest byte in the RAX is set
        // as the value of the returned boolean flag. As a result, 
        // AND the value in RAX with 0xFF to get the lowest byte in RAX.
        m_code.AND(RAX, 0xFF);
        m_code.XOR(RAX, 0);
        m_code.JNZ(earlyTerminate, X64::LONG);

        m_code.PlaceLabel(bottomOfLoop);

        // Bottom of loop
        m_code.INC(RCX);                       // Increment the loop counter.
        m_code.JMP(topOfLoop, X64::LONG);

        m_code.PlaceLabel(earlyTerminate);

        // DESIGN NOTE: Use the higher 32 bits in RAX to store a value to indicate
        // whether the evaluation should be early terminated. A value of 1 in the higher
        // 32 bits in RAX indicates the evaluation should be early terminated in the
        // outer loop. A value of zero in the higher 32 bits in RAX indicates the 
        // evaluation should continue in the outer loop.
        //
        // Use the lower 32 bits in RAX to store the number of quadwords have not been
        // scanned in the inner loop. In practise, the number of quadword in any
        // slice should be smaller than 2^32, thus using 32 bits is enough.
        m_code.MOV(RAX, 1ULL);
        m_code.SHL(RAX, 32);

        m_code.PlaceLabel(exitLoop);
        
        // RCX contains the number of iterations that have not been scanned.
        m_code.MOV(RDX, m_code.ParameterHome(2));
        m_code.SUB(RDX, RCX);        
        m_code.OR(RAX, RDX);
    }


    void MatchTreeCodeGenerator::EmitFinishIteration()
    {
        // Save volatile registers
        m_code.PUSH(RCX);
        m_code.PUSH(RDX);
        m_code.PUSH(R8);
        m_code.PUSH(R9);
        m_code.PUSH(R10);
        m_code.PUSH(R11);
        m_pushCount += 6;

        // Plan to allocate one slot for ProcessGroupHelper's one parameter.
        int parameterCount = 2;
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
        m_code.MOV(RDX, m_code.ParameterHome(0));     // Second parameter is the slice pointer.
        m_code.MOV(RDX, QWORDPTR[RDX]);

        m_code.MOV(RCX, m_code.ParameterHome(4));     // First parameter is the IResultsProcessor.

        m_code.MOV(RAX, m_code.ParameterHome(5));       // This is the address of FinishIterationHelper passed as a parameter
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


    void MatchTreeCodeGenerator::EmitFunctionReturn()
    {
        m_code.EmitEpilogue();
    }
}
