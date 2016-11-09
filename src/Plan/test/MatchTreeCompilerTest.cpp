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

#include <iostream>

#include "gtest/gtest.h"

#include "CompileNode.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/CodeGenHelpers.h"
#include "NativeJIT/Function.h"
#include "NativeJIT/CodeGen/Register.h"
#include "Temporary/Allocator.h"

using namespace NativeJIT;


namespace BitFunnel
{
    //*************************************************************************
    //
    //
    //
    //*************************************************************************
    class MatchTreeCompiler
    {
    public:
        MatchTreeCompiler(ExecutionBuffer & codeAllocator,
                          Allocator & allocator,
                          CompileNode const & tree);

        size_t Run(size_t slicecount,
                   char * const * slicebuffers,
                   size_t iterationsperslice,
                   ptrdiff_t const * rowoffsets);

    private:
        friend class MatcherNode;

        static size_t CallbackHelper(/*MatchTreeCompiler& node, */size_t value);

//        typedef void(*Callback)(MatchTreeCompiler& node, size_t value);
        typedef size_t (*Callback)(size_t value);

        struct Parameters
        {
        public:
            size_t m_sliceCount;
            char * const * m_sliceBuffers;
            size_t m_iterationsPerSlice;
            ptrdiff_t const * m_rowOffsets;
            Callback m_callback;
        };
        static_assert(std::is_standard_layout<Parameters>::value,
                      "Parameters must be standard layout.");

        FunctionBuffer m_code;

        typedef Function<size_t, Parameters const *> Prototype;
        Prototype::FunctionType m_function;
    };


    //*************************************************************************
    //
    // MatcherNode
    //
    //*************************************************************************
    class MatcherNode : public NativeJIT::Node<size_t>
    {
    public:
        MatcherNode(MatchTreeCompiler::Prototype& expression,
                    CompileNode const & matchTree);

        virtual ExpressionTree::Storage<size_t> CodeGenValue(ExpressionTree& tree) override;

        virtual void Print(std::ostream& out) const override;

    private:
        void EmitRegisterInitialization(ExpressionTree& tree);
        void EmitOuterLoop(ExpressionTree& tree);
        void EmitInnerLoop(ExpressionTree& tree);
        void EmitFinishIteration(ExpressionTree& tree);

        CompileNode const & m_matchTree;

        Register<8u, false> m_param1;
        Register<8u, false> m_return;

        Storage<size_t> m_sliceCount;
        Storage<char * const *> m_sliceBuffers;
        Storage<size_t> m_iterationsPerSlice;
        Storage<ptrdiff_t const *> m_rowOffsets;
        Storage<MatchTreeCompiler::Callback> m_callback;
        Storage<size_t> m_innerLoopLimit;
        Storage<size_t> m_matchFound;
        Storage<size_t> m_temp;
    };


    MatcherNode::MatcherNode(MatchTreeCompiler::Prototype& expression,
                             CompileNode const & matchTree)
      : Node(expression),
        m_matchTree(matchTree)
    {
    }


    ExpressionTree::Storage<size_t> MatcherNode::CodeGenValue(ExpressionTree& tree)
    {
        EmitRegisterInitialization(tree);
        EmitOuterLoop(tree);

        auto result = tree.Direct<size_t>();
        return result;
    }


    template <typename OBJECT, typename FIELD>
    Storage<FIELD> Initialize(ExpressionTree& tree, Register <8u, false> base, FIELD OBJECT::*field)
    {
        auto & code = tree.GetCodeGenerator();
        auto storage = tree.Temporary<FIELD>();
        int32_t offset =
            static_cast<int32_t>(reinterpret_cast<uint64_t>(&((static_cast<OBJECT*>(nullptr))->*field)));
        code.Emit<OpCode::Mov>(rax, base, offset);
        CodeGenHelpers::Emit<NativeJIT::OpCode::Mov>(code, storage, rax);
        return storage;
    }


    // RAX: Scratch register
    // RBX: Accumulator
    // RCX: Loop counter at inner loop root, offset elsewhere in traversal
    //      Encoded as (&SLICE_POINTER >> 3) + offset.
    // RDX: Current slice pointer.
    // RSI: Pointer to array of all row offset pointers.
    // RDI: &SLICE_POINTER >> 3
    // R8-R15: Register row pointers.


    void MatcherNode::EmitRegisterInitialization(ExpressionTree& tree)
    {
        // Abstract away the ABI differences here.
#if BITFUNNEL_PLATFORM_WINDOWS
        m_param1 = rcx;
        m_return = rax;
#else
        m_param1 = rdi;
        m_return = rax;
#endif

        // Initialize member variables for copy of Parameters structure.
        m_sliceCount = Initialize(tree, m_param1, &MatchTreeCompiler::Parameters::m_sliceCount);
        m_sliceBuffers = Initialize(tree, m_param1, &MatchTreeCompiler::Parameters::m_sliceBuffers);
        m_iterationsPerSlice = Initialize(tree, m_param1, &MatchTreeCompiler::Parameters::m_iterationsPerSlice);
        m_rowOffsets = Initialize(tree, m_param1, &MatchTreeCompiler::Parameters::m_rowOffsets);
        m_callback = Initialize(tree, m_param1, &MatchTreeCompiler::Parameters::m_callback);
        m_innerLoopLimit = tree.Temporary<size_t>();
        m_matchFound = tree.Temporary<size_t>();
        m_temp = tree.Temporary<size_t>();

        // Initialize row pointers.
        // TODO
    }


    void MatcherNode::EmitOuterLoop(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        auto topOfLoop = code.AllocateLabel();
        auto bottomOfLoop = code.AllocateLabel();


        //
        // Top of loop
        //
        code.PlaceLabel(topOfLoop);

        // Check if the slice count (loop counter) reaches zero.
        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_sliceCount);
        code.Emit<OpCode::Or>(rax, rax);
        code.EmitConditionalJump<JccType::JZ>(bottomOfLoop);

        // RDI has the current slice pointer, expressed in the format of 
        // the number of quadwords from address 0 to the address of the 
        // current slice pointer(i.e. shifted right by 3).
        CodeGenHelpers::Emit<OpCode::Mov>(code, rdi, m_sliceBuffers);
        code.Emit<OpCode::Mov>(rdi, rdi, 0);
        code.EmitImmediate<OpCode::Shr>(rdi, static_cast<uint8_t>(3u));

        EmitInnerLoop(tree);

        // Decrement the slice count by 1.
        // TODO: Implement OpCode::Dec
        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_sliceCount);
        code.EmitImmediate<OpCode::Sub>(rax, 1);
        CodeGenHelpers::Emit<OpCode::Mov>(code, m_sliceCount, rax);

        // Advance to the next slice.
        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_sliceBuffers);
        code.EmitImmediate<OpCode::Add>(rax, 8);
        CodeGenHelpers::Emit<OpCode::Mov>(code, m_sliceBuffers, rax);

        code.Jmp(topOfLoop);

        //
        // Bottom of loop
        //
        code.PlaceLabel(bottomOfLoop);
    }


    void MatcherNode::EmitInnerLoop(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        auto topOfLoop = code.AllocateLabel();
        auto bottomOfLoop = code.AllocateLabel();
        auto exitLoop = code.AllocateLabel();

        // Initialize loop counter and limit.
        // Loop counter starts at the current slice pointer, expressed as the 
        // number of quadwords from address 0 to the address of the current slice
        // pointer(i.e. byte address shifted right by 3).
        // Loop limit is equal to the counter plus the number of quadwords in a slice.
        // Initialize current quadword and limit.
        //   rcx: current quadword - starts at slice pointer expressed in
        //        from address 0 (i.e slice >> 3).
        //   rdx: quadword limit - equal to (slice >> 3) + quadwords in a slice.
        code.Emit<OpCode::Mov>(rcx, rdi);
        CodeGenHelpers::Emit<OpCode::Mov>(code, rdx, m_iterationsPerSlice);
        code.Emit<OpCode::Add>(rdx, rcx);
        CodeGenHelpers::Emit<OpCode::Mov>(code, m_innerLoopLimit, rdx);


        // Use rdx to store the slice pointer for the current slice.
        CodeGenHelpers::Emit<OpCode::Mov>(code, rdx, m_sliceBuffers);
        code.Emit<OpCode::Mov>(rdx, rdx, 0);


        //
        // Top of loop
        //
        code.PlaceLabel(topOfLoop);

        // Exit when loop counter rcx == m_innerLoopLimit
        CodeGenHelpers::Emit<OpCode::Cmp>(code, rcx, m_innerLoopLimit);
        code.EmitConditionalJump<JccType::JE>(exitLoop);    // TODO: Original code passed X64::Long.

        //
        // Body of loop
        //

        // TODO: Handle case where there are no rows.

        // Clear Local(0) to indicate that no matches have been found on this iteration.
        code.Emit<OpCode::Xor>(rax, rax);
        CodeGenHelpers::Emit<OpCode::Mov>(code, m_matchFound, rax);

        //root.Compile(*this);

        CodeGenHelpers::Emit<OpCode::Mov>(code, m_temp, rcx);

//        CodeGenHelpers::Emit<OpCode::Mov>(code, m_param1, m_sliceCount);
        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_callback);
        code.Emit<OpCode::Call>(rax);

        CodeGenHelpers::Emit<OpCode::Mov>(code, rcx, m_temp);


        // If at least one match was found in this iteration, call FinishIteration.
        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_matchFound);
        code.Emit<OpCode::Or>(rax, rax);
        code.EmitConditionalJump<JccType::JZ>(bottomOfLoop);    // TODO: Original code passed X64::Long.

        EmitFinishIteration(tree);

        //
        // Bottom of loop
        //
        code.PlaceLabel(bottomOfLoop);
        code.EmitImmediate<OpCode::Add>(rcx, 1);                // Increment current.
        code.Jmp(topOfLoop);                                    // TODO: Original code passed X64::Long.


        code.PlaceLabel(exitLoop);
    }


    void MatcherNode::EmitFinishIteration(ExpressionTree& /*tree*/)
    {

    }


    void MatcherNode::Print(std::ostream& out) const
    {
        this->PrintCoreProperties(out, "MatcherNode");

//        out << ", scorePlan = " << m_left.GetId();
    }


    //*************************************************************************
    //
    // MatchTreeCompiler
    //
    //*************************************************************************
    MatchTreeCompiler::MatchTreeCompiler(ExecutionBuffer & codeAllocator,
                                         Allocator & allocator,
                                         CompileNode const & tree)
      : m_code(codeAllocator, 8192)
    {
        Function<size_t, Parameters const *> expression(allocator, m_code);
        expression.EnableDiagnostics(std::cout);

        auto & node = expression.PlacementConstruct<MatcherNode>(expression, tree);
        m_function = expression.Compile(node);
    }


    size_t MatchTreeCompiler::Run(size_t sliceCount,
                                  char * const * sliceBuffers,
                                  size_t iterationsPerSlice,
                                  ptrdiff_t const * rowOffsets)
    {
        MatchTreeCompiler::Parameters parameters = {
            sliceCount,
            sliceBuffers,
            iterationsPerSlice,
            rowOffsets,
            &CallbackHelper
        };

        return m_function(&parameters);
    }


    size_t MatchTreeCompiler::CallbackHelper(//MatchTreeCompiler& /*node*/,
                                             size_t value)
    {
        std::cout
            << "CallbackHelper("
            << value
            << ")"
            << std::endl;

        return 1234567ull;
    }


    TEST(MatchTreeCompiler , Placeholder)
    {
        // Create allocator and buffers for pre-compiled and post-compiled code.
        ExecutionBuffer codeAllocator(8192);
        Allocator allocator(8192);

        CompileNode const * node = nullptr;

        MatchTreeCompiler compiler(codeAllocator,
                                   allocator,
                                   *node);

        std::vector<char *> m_sliceBuffers(3, 0);
        std::vector<ptrdiff_t> m_rowOffsets(2, 0);

        auto result = compiler.Run(m_sliceBuffers.size(),
                                   m_sliceBuffers.data(),
                                   m_rowOffsets.size(),
                                   m_rowOffsets.data());
        //auto result = compiler.Run(3ull,
        //                           nullptr,
        //                           0ull,
        //                           nullptr);

        std::cout << "Result = " << result << std::endl;
    }
}
