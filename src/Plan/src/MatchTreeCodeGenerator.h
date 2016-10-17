#pragma once

#include "MachineCodeGenerator.h"   // Inherits from MachineCodeGenerator.


namespace BitFunnel
{
    class CompileNode;

    //*************************************************************************
    //
    // MatchTreeCodeGenerator extends MachineCodeGenerator by adding
    // functionality specific to the compilation of the matching algorithm.
    //
    // This functionality includes
    //   1. Initializing registers R8..R15 with row pointers.
    //   2. Emitting the main loop.
    //   3. Generating the calls to the FinishIteration() callback.
    //   4. Generating the function return.
    //
    // The usage pattern is
    //   1. Translate the matching plan into a CompileNode tree.
    //   2. Construct a RegisterAllocator based on the CompileNode tree.
    //   3. Get a MatchingEngine from a pool. The MatchingEngine holds the
    //      X64FunctionGenerator which will be passed to the
    //      MatchTreeCodeGenerator.
    //   4. Construct the MatchTreeCodeGenerator.
    //   5. Generate X64 code by passing the CompileNode tree to
    //      MatchTreeCodeGenerator::GenerateX64Code().
    //   6. Either invoke the X64 code via MatchingEngine::Run() method or
    //      extract the code via MatchingEngine::GetFunction().
    //   7. Return the MatchingEngine to the pool.
    //
    //*************************************************************************
    class MatchTreeCodeGenerator : public MachineCodeGenerator
    {
    public:
        // Constructs a MatchTreeCodeGenerator which generates machine code
        // using the supplied X64FunctionGenerator. The registers parameter
        // provides a register allocator that defines register assignments for
        // some rows.
        MatchTreeCodeGenerator(RegisterAllocator const & registers,
                               X64::X64FunctionGenerator& code,
                               unsigned maxIterationsScannedBetweenTerminationChecks);

        // Translates the CompileNode tree into X64 machine code.
        void GenerateX64Code(CompileNode const & root);

    private:
        // Generates the function prologue which saves registers, write some
        // parameters to their home slots, and initializes registers used for
        // row pointers.
        void EmitRegisterInitialization();

        // Generates the outer matching loop over the set of slices.
        void EmitOuterLoop(CompileNode const & root);

        // Generates the inner matching loop over one single slice.
        void EmitInnerLoop(CompileNode const & root);

        // Generates the callback from the matcher to FinishIterationHelper().
        void EmitFinishIteration();

        // Generates code at the end of the function which restores registers
        // and returns.
        void EmitFunctionReturn();

        // The number of slices that should be processed until a call to the 
        // iteration finish event is required. This is meant to ensure the 
        // termination check is performed even when there are no matches.
        //
        // There's a tradeoff between performance and maximum overrun of the
        // query timeout here. If we set a low value (e.g. every other iteration),
        // the termination helper will get called very frequently, which will
        // reduce the time spent matching. It will however guarantee the engine
        // will stop executing very shortly after the termination has been met.
        // Putting a large value will lower the termination check overhead, but
        // could lead to termination of execution much longer past the timeout. 
        //
        // Setting this to 0 will disable the feature.
        // Setting this to a number of iterations larger than what a slice can
        // contain will enable the feature, but the condition will never be met,
        // essentially wasting CPU.
        const unsigned m_maxIterationsScannedBetweenTerminationChecksMask;

        // This stores the orginal setting passing in by the users. The mask 
        // cannot be used to differentiate between "no termination check" and
        // "check after each iteration" scenarios.
        const unsigned m_maxIterationsScannedBetweenTerminationChecks;
    };
}
