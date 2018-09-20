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


#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "QueryResources.h"


namespace BitFunnel
{
    QueryResources::QueryResources(size_t treeAllocatorBytes,
                                   size_t codeAllocatorBytes)
      : m_matchTreeAllocator(new BitFunnel::Allocator(treeAllocatorBytes)),
        m_expressionTreeAllocator(new NativeJIT::Allocator(treeAllocatorBytes)),
        m_codeAllocator(new NativeJIT::ExecutionBuffer(codeAllocatorBytes)),
        m_countCacheLines(false)
    {
        m_code.reset(new NativeJIT::FunctionBuffer(*m_codeAllocator,
                                                   static_cast<unsigned>(codeAllocatorBytes)));
    }


    void QueryResources::EnableCacheLineCounting()
    {
        m_countCacheLines = true;
    }


    void QueryResources::Reset()
    {
        m_matchTreeAllocator->Reset();
        m_expressionTreeAllocator->Reset();
        // WARNING: Do not reset m_codeAllocator. It is used to provision m_code.
        m_code->Reset();
    }
}
