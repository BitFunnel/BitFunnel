#pragma once

#include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.


// namespace X64
// {
//     class X64FunctionGenerator;
// }


namespace BitFunnel
{
    class IAllocator;
    class IPlanRows;
    class ISimpleIndex;
    class IThreadResources;
    class TermPlan;
    // class X64::X64FunctionGenerator;

    class QueryPlanner : public NonCopyable
    {
    public:
        // Constructs a QueryPlanner with the specified resources.
        QueryPlanner(TermPlan const & termPlan,
                     unsigned targetRowCount,
                     ISimpleIndex const & index,
                     // IThreadResources& threadResources,
                     IAllocator& allocator,
                     IDiagnosticStream* diagnosticStream);
                     // bool generateNonBodyPlan,
                     // unsigned maxIterationsScannedBetweenTerminationChecks);

        //
        // IQueryPlanner methods.
        //

        // const CompiledFunction GetMatchingFunction() const;

        IPlanRows const & GetPlanRows() const;

    private:

        // // Wrapper class for X64FunctionGenerator to manage the
        // // allocate and release of the X64FunctionGenerator object in
        // // ThreadResources.
        // // The goal of this class is to be an RAII wrapper to protect
        // // against throws in the constructor of QueryPlanner.
        // class X64FunctionGeneratorWrapper : public NonCopyable
        // {
        // public:
        //     // The constructor takes a X64FunctionGenerator from thread resources.
        //     X64FunctionGeneratorWrapper(IThreadResources& threadResources);

        //     // The destructor return the X64FunctionGenerator back to the thread resources.
        //     ~X64FunctionGeneratorWrapper();

        //     operator X64::X64FunctionGenerator&() const;

        // private:
        //     // The X64FunctionGenerator maintains the executable buffers of X64
        //     X64::X64FunctionGenerator& m_code;

        //     // Stored so that the X64FunctionGenerator can be released during destruction
        //     IThreadResources& m_threadResources;
        // };

        // The X64FunctionGeneratorWrapper which wraps a x64FunctionGenerator allocated from
        // thread resources. It also is responsible for release the wrapped x64FunctionGenerator.
        // X64FunctionGeneratorWrapper m_x64FunctionGeneratorWrapper;

        IPlanRows const * m_planRows;

        // The maximum number of iterations that can be performed before a termination
        // check is mandatory. Details can be found in the MatchTreeCodeGenerator.
        // const unsigned m_maxIterationsScannedBetweenTerminationChecks;

        // First available row pointer register is R8.
        // TODO: is this valid on all platforms or only on Windows?
        static const unsigned c_registerBase = 8;

        // Row pointers stored in the eight registers R8..R15.
        // TODO: is this valid on all platforms or only on Windows?
        static const unsigned c_registerCount = 8;
    };
}
