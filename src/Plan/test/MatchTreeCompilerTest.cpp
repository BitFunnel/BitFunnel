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
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"

using NativeJIT::Allocator;
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
    class MatcherNode : public NativeJIT::Node<int>
    {
    public:
        MatcherNode(ExpressionTree& expression,
                    CompileNode const & matchTree);

        virtual ExpressionTree::Storage<int> CodeGenValue(ExpressionTree& tree) override;

        virtual void Print(std::ostream& out) const override;

    private:
        CompileNode const & m_matchTree;
    };


    MatcherNode::MatcherNode(ExpressionTree& expression,
                             CompileNode const & matchTree)
      : Node(expression),
        m_matchTree(matchTree)
    {
    }


    ExpressionTree::Storage<int> MatcherNode::CodeGenValue(ExpressionTree& tree)
    {
//        return Storage<size_t>::ForImmediate(tree, static_cast<int>(12345));
        return tree.Immediate(12345);
    }


    void MatcherNode::Print(std::ostream& out) const
    {
        this->PrintCoreProperties(out, "MatcherNode");

//        out << ", scorePlan = " << m_left.GetId();
    }


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

        size_t Run();

        //size_t Run(size_t sliceCount,
        //           char * const * sliceBuffers,
        //           size_t iterationsPerSlice,
        //           ptrdiff_t const * rowOffsets);

    private:
        static void CallbackHelper(MatchTreeCompiler& node, size_t value);

        typedef void (*Callback)(MatchTreeCompiler& node, size_t value);

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
        Function<int, Parameters const *>::FunctionType m_function;
    };


    MatchTreeCompiler::MatchTreeCompiler(ExecutionBuffer & codeAllocator,
                                         Allocator & allocator,
                                         CompileNode const & tree)
      : m_code(codeAllocator, 8192)
    {
        Function<int, Parameters const *> expression(allocator, m_code);
        auto & node = expression.PlacementConstruct<MatcherNode>(expression, tree);
        m_function = expression.Compile(node);
    }



    size_t MatchTreeCompiler::Run()
    {
        MatchTreeCompiler::Parameters parameters = {
            0ull,
            nullptr,
            0ull,
            nullptr,
            &CallbackHelper
        };

        return m_function(&parameters);
    }


    //size_t MatchTreeCompiler::Run(size_t sliceCount,
    //                              char * const * sliceBuffers,
    //                              size_t iterationsPerSlice,
    //                              ptrdiff_t const * rowOffsets)
    //{
    //    MatchTreeCompiler::Parameters parameters = {
    //        sliceCount,
    //        sliceBuffers,
    //        iterationsPerSlice,
    //        rowOffsets,
    //        &CallbackHelper
    //    };

    //    return m_function(parameters);
    //}


    void MatchTreeCompiler::CallbackHelper(MatchTreeCompiler& /*node*/,
                                           size_t /*value*/)
    {
    }



    //*************************************************************************
    //
    //
    //
    //*************************************************************************
    TEST(MatchTreeCompiler , Placeholder)
    {
        // Create allocator and buffers for pre-compiled and post-compiled code.
        ExecutionBuffer codeAllocator(8192);
        Allocator allocator(8192);

        CompileNode const * node = nullptr;

        MatchTreeCompiler compiler(codeAllocator,
                                   allocator,
                                   *node);

        auto result = compiler.Run();

        std::cout << "Result = " << result << std::endl;

        //// Create the factory for expression nodes.
        //// Our area expression will take a single float parameter and return a float.
        //Function<float, float> expression(allocator, code);

        //// Multiply input parameter by itself to get radius squared.
        //auto & rsquared = expression.Mul(expression.GetP1(), expression.GetP1());

        //// Multiply by PI.
        //const float  PI = 3.14159265358979f;
        //auto & area = expression.Mul(rsquared, expression.Immediate(PI));

        //// Compile expression into a function.
        //auto function = expression.Compile(area);

        //// Now run our expression!
        //float radius = 2.0;
        //std::cout << "The area of a circle with radius " << radius
        //    << " is " << function(radius) << "." << std::endl;
    }
}
