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

#include <memory>                               // std::unique_ptr embedded.

#include "BitFunnel/Allocators/IAllocator.h"    // Template parameter.
#include "NativeJIT/CodeGen/ExecutionBuffer.h"  // Template parameter.
#include "NativeJIT/CodeGen/FunctionBuffer.h"   // Template parameter.
#include "Temporary/Allocator.h"                // Template parameter.


namespace BitFunnel
{
    class ISimpleIndex;

    class QueryResources
    {
    public:
        QueryResources(size_t treeAllocatorBytes = 1ull << 16,
                       size_t codeAllocatorBytes = 1ull << 16);

        void EnableCacheLineCounting();

        virtual void Reset();

        IAllocator & GetMatchTreeAllocator() const
        {
            return *m_matchTreeAllocator;
        }

        NativeJIT::Allocator & GetExpressionTreeAllocator() const
        {
            return *m_expressionTreeAllocator;
        }

        NativeJIT::ExecutionBuffer & GetCodeAllocator() const
        {
            return *m_codeAllocator;
        }

        NativeJIT::FunctionBuffer & GetCode() const
        {
            return *m_code;
        }

        bool GetCountCacheLines()
        {
            return m_countCacheLines;
        }

    private:
        std::unique_ptr<IAllocator> m_matchTreeAllocator;
        std::unique_ptr<NativeJIT::Allocator> m_expressionTreeAllocator;
        std::unique_ptr<NativeJIT::ExecutionBuffer> m_codeAllocator;
        std::unique_ptr<NativeJIT::FunctionBuffer> m_code;
        bool m_countCacheLines;
    };
}
