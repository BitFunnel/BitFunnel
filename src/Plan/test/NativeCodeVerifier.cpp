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

#include <iomanip>
#include <iostream>

#include "gtest/gtest.h"

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Plan/RowMatchNode.h"
#include "BitFunnel/Term.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "MatchTreeCompiler.h"
#include "NativeCodeVerifier.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "CompileNode.h"
#include "RegisterAllocator.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    NativeCodeVerifier::NativeCodeVerifier(ISimpleIndex const & index,
                                           Rank initialRank)
      : CodeVerifierBase(index, initialRank)
    {
    }


    // TODO: move this method somewhere.
    static uint64_t lzcnt(uint64_t value)
    {
#ifdef _MSC_VER
        return __lzcnt64(value);
#elif __LZCNT__
        return __lzcnt64(value);
#else
        // DESIGN NOTE: this is undefined if the input operand i 0. We currently
        // only call lzcnt in one place, where we've prevously gauranteed that
        // the input isn't 0. This is not safe to use in the general case.
        return static_cast<uint64_t>(__builtin_clzll(value));
#endif
    }

    // The class is configured by adding a sequence of of records
    // describing the expected interactions betweeen the matcher and its
    // IResultsProcessor. Each call to Add() corresponds to expectation
    // that the matcher will invoke IResultsProcessor::AddResult(). The
    // accumulator and offset parameters to Add() are the expected
    // parameters to AddResult().
    //
    // The usage pattern for IResultsProcessor is a sequence of calls
    // to AddResult() interspersed with calls to FinishIteration().
    // The call to FinishIteration() provides the void* slice buffer
    // pointer that is applicable to the sequence of calls to AddResult()
    // since the previous call to FinishIteration() (or the start of
    // the matching algorithm if there was no previous call to
    // FinishIteration().
    //
    // The third parameter of the Add() method indicates the slice
    // that expected to be passed on the next call to FinishIteration.
    // The slice parameter is a size_t index into an array of slice
    // buffer pointers associated with the index.
    //
    void NativeCodeVerifier::ExpectResult(uint64_t accumulator,
                                          size_t offset,
                                          size_t sliceNumber)
    {
        //// TODO: Remove temporary debugging output.
        //std::cout
        //    << "ExpectResult("
        //    << std::hex << accumulator << std::dec
        //    << ", "
        //    << offset
        //    << ", "
        //    << sliceNumber
        //    << ")"
        //    << std::endl;

        size_t bitPos = 63;
        void * sliceBuffer = m_slices[sliceNumber];

        while (accumulator != 0)
        {
            uint64_t count = lzcnt(accumulator);
            accumulator <<= count;
            bitPos -= count;

            DocIndex docIndex = offset * c_bitsPerQuadword + bitPos;
            DocumentHandle handle =
                Factories::CreateDocumentHandle(sliceBuffer, docIndex);

            if (handle.IsActive())
            {
                DocId id = handle.GetDocId();
                //// TODO: Remove temporary debugging output.
                //std::cout << "  " << id << std::endl;

                if (m_expected.find(id) != m_expected.end())
                {
                    std::cout << "  Duplicate id " << id << std::endl;
                }

                m_expected.insert(id);
            }

            accumulator <<= 1;
            --bitPos;
        }
    }


    void NativeCodeVerifier::Verify(char const * codeText)
    {
        // Create allocator and buffers for pre-compiled and post-compiled code.
        NativeJIT::ExecutionBuffer codeAllocator(8192);
        NativeJIT::Allocator treeAllocator(8192);
        BitFunnel::Allocator allocator(2048);

        std::stringstream input(codeText);

        TextObjectParser parser(input, allocator, &CompileNode::GetType);
        CompileNode const & compileNodeTree = CompileNode::Parse(parser);

        RegisterAllocator registers(compileNodeTree,
                                    8,
                                    8,
                                    7,
                                    allocator);


        MatchTreeCompiler compiler(codeAllocator,
                                   treeAllocator,
                                   compileNodeTree,
                                   registers);

        ResultsBuffer results(m_index.GetIngestor().GetDocumentCount());

        compiler.Run(m_slices.size(),
                     m_slices.data(),
                     GetIterationsPerSlice(),
                     m_rowOffsets.data(),
                     results);

        for (auto result : results)
        {
            DocumentHandle handle =
                Factories::CreateDocumentHandle(result.m_slice, result.m_index);

            if (handle.IsActive())
            {
                DocId doc = handle.GetDocId();

                // TODO: Remove temporary debugging output.
                //std::cout
                //    << "  ==> " << doc << std::endl;

                m_observed.insert(doc);
            }
        }

        ASSERT_EQ(m_expected.size(), m_observed.size())
            << "Inconsistent match counts.";

        for (DocId d : m_expected)
        {
            auto it = m_observed.find(d);
            ASSERT_TRUE(it != m_observed.end())
                << "Expected docId "
                << d
                << " not found.";
        }
    }
}
