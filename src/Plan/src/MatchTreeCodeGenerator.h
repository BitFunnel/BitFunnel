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

#include <stddef.h>     // size_t, ptrdiff_t parameters.

#include "NativeJIT/CodeGen/FunctionBuffer.h"   // FunctionBuffer embedded.
#include "NativeJIT/Function.h"                 // Function in typedef.


namespace NativeJIT
{
    class Allocator;
    class ExecutionBuffer;
    class FunctionBuffer;
};

using namespace NativeJIT;

namespace BitFunnel
{
    class CompileNode;
    class DocumentHandle;
    class RegisterAllocator;


    template <typename OBJECT, typename FIELD>
    constexpr int32_t OffsetOf(FIELD OBJECT::*field)
    {
        return static_cast<int32_t>(reinterpret_cast<uint64_t>(&((static_cast<OBJECT*>(nullptr))->*field)));
    }

#define OFFSET_OF(object, field) \
static_cast<int32_t>(reinterpret_cast<uint64_t>(&((static_cast<object*>(nullptr))->field)))


    //*************************************************************************
    //
    // MatcherNode
    //
    // A NativeJIT::Node that implements the BitFunnel matching algorithm.
    //
    //*************************************************************************
    class MatcherNode : public NativeJIT::Node<size_t>
    {
    public:
        typedef size_t(*Callback)(size_t value);

        struct Record
        {
            void * m_buffer;
            size_t m_id;
        };

        struct Parameters
        {
        public:
            // Inputs
            size_t m_sliceCount;
            char * const * m_sliceBuffers;
            size_t m_iterationsPerSlice;
            ptrdiff_t const * m_rowOffsets;
            Callback m_callback;

            // Dedupe
            size_t m_dedupe[65];

            // Matches
            size_t m_capacity;
            size_t m_matchCount;
            Record* m_matches;
        };
        static_assert(std::is_standard_layout<Parameters>::value,
                      "Parameters must be standard layout.");

        typedef Function<size_t, Parameters const *> Prototype;
        Prototype::FunctionType m_function;

        MatcherNode(Prototype& expression,
                    CompileNode const & compileNodeTree,
                    RegisterAllocator const & registers);

        virtual ExpressionTree::Storage<size_t> CodeGenValue(ExpressionTree& tree) override;

        virtual void Print(std::ostream& out) const override;

        static const int32_t m_sliceCount = OFFSET_OF(Parameters, m_sliceCount);
        static const int32_t m_sliceBuffers = OFFSET_OF(Parameters, m_sliceBuffers);
        static const int32_t m_iterationsPerSlice = OFFSET_OF(Parameters, m_iterationsPerSlice);
        static const int32_t m_rowOffsets = OFFSET_OF(Parameters, m_rowOffsets);
        static const int32_t m_callback = OFFSET_OF(Parameters, m_callback);
        static const int32_t m_dedupe = OFFSET_OF(Parameters, m_dedupe);
        static const int32_t m_capacity = OFFSET_OF(Parameters, m_capacity);
        static const int32_t m_matchCount = OFFSET_OF(Parameters, m_matchCount);
        static const int32_t m_matches = OFFSET_OF(Parameters, m_matches);


    private:
        void EmitRegisterInitialization(ExpressionTree& tree);
        void EmitOuterLoop(ExpressionTree& tree);
        void EmitInnerLoop(ExpressionTree& tree);
        void EmitFinishIteration(ExpressionTree& tree);
        void EmitStoreMatch(ExpressionTree & tree);

        CompileNode const & m_compileNodeTree;
        RegisterAllocator const & m_registers;

        Register<8u, false> m_param1;
        Register<8u, false> m_return;

        Storage<size_t> m_innerLoopLimit;
    };


    //*************************************************************************
    //
    // MatchTreeCompiler
    //
    //*************************************************************************
    class MatchTreeCompiler
    {
    public:
        MatchTreeCompiler(ExecutionBuffer & codeAllocator,
                          NativeJIT::Allocator & treeAllocator,
                          CompileNode const & tree,
                          RegisterAllocator const & registers);

        size_t Run(size_t slicecount,
                   char * const * slicebuffers,
                   size_t iterationsperslice,
                   ptrdiff_t const * rowoffsets);

    private:
        static size_t CallbackHelper(/*MatchTreeCompiler& node, */size_t value);

        FunctionBuffer m_code;

        MatcherNode::Prototype::FunctionType m_function;
    };
}
