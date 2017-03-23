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


#include <new>

#include "BitFunnel/Allocators/IAllocator.h"
#include "CompileNode.h"
#include "LoggerInterfaces/Logging.h"
#include "RankZeroCompiler.h"
#include "RowMatchNode.h"

// TODO: get rid of dynamic cast?

namespace BitFunnel
{
    RankZeroCompiler::RankZeroCompiler(IAllocator& allocator)
        : m_allocator(allocator)
    {
    }


    CompileNode const & RankZeroCompiler::Compile(RowMatchNode const & node)
    {
        CompileNode const * result = nullptr;

        switch (node.GetType())
        {
        case RowMatchNode::AndMatch:
            result = new (m_allocator.Allocate(sizeof(CompileNode::AndTree)))
                         CompileNode::AndTree(Compile(dynamic_cast<RowMatchNode::And const &>(node).GetLeft()),
                                              Compile(dynamic_cast<RowMatchNode::And const &>(node).GetRight()));
            break;
        case RowMatchNode::NotMatch:
            result = new (m_allocator.Allocate(sizeof(CompileNode::Not)))
                         CompileNode::Not(Compile(dynamic_cast<RowMatchNode::Not const &>(node).GetChild()));
            break;
        case RowMatchNode::OrMatch:
            result = new (m_allocator.Allocate(sizeof(CompileNode::OrTree)))
                         CompileNode::OrTree(Compile(dynamic_cast<RowMatchNode::Or const &>(node).GetLeft()),
                                             Compile(dynamic_cast<RowMatchNode::Or const &>(node).GetRight()));
            break;
        case RowMatchNode::RowMatch:
            {
                AbstractRow const & row = dynamic_cast<RowMatchNode::Row const &>(node).GetRow();
                result = new (m_allocator.Allocate(sizeof(CompileNode::LoadRow)))
                             CompileNode::LoadRow(row);
            }
            break;
        default:
            LogAbortB("Unsupported node type.");
        }

        return *result;
    }
}
