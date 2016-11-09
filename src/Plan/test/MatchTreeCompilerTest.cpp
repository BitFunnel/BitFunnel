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

using NativeJIT::Allocator;
//using namespace NativeJIT::CodeGenHelpers;
using NativeJIT::ExecutionBuffer;
using NativeJIT::ExpressionTree;
using NativeJIT::Function;
using NativeJIT::FunctionBuffer;
using NativeJIT::Storage;


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

        CompileNode const & m_matchTree;

        Register<8u, false> m_param1;
        Register<8u, false> m_return;

        Storage<size_t> m_sliceCount;
        Storage<char * const *> m_sliceBuffers;
        Storage<size_t> m_iterationsPerSlice;
        Storage<ptrdiff_t const *> m_rowOffsets;
        Storage<MatchTreeCompiler::Callback> m_callback;
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
        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_sliceCount);
        code.Emit<OpCode::Or>(rax, rax);
        code.EmitConditionalJump<JccType::JZ>(bottomOfLoop);

        EmitInnerLoop(tree);

        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_sliceCount);
        code.EmitImmediate<OpCode::Sub>(rax, 1);
        CodeGenHelpers::Emit<OpCode::Mov>(code, m_sliceCount, rax);

        code.Jmp(topOfLoop);

        //
        // Bottom of loop
        //
        code.PlaceLabel(bottomOfLoop);
    }


    void MatcherNode::EmitInnerLoop(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        CodeGenHelpers::Emit<OpCode::Mov>(code, m_param1, m_sliceCount);
        CodeGenHelpers::Emit<OpCode::Mov>(code, rax, m_callback);
        code.Emit<OpCode::Call>(rax);
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

        auto result = compiler.Run(3ull,
                                   nullptr,
                                   0ull,
                                   nullptr);

        std::cout << "Result = " << result << std::endl;
    }
}
