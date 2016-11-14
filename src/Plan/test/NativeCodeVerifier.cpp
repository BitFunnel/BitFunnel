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

#include "Allocator.h"
#include "BitFunnel/IDiagnosticStream.h" // TODO: remove.
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Term.h"
#include "BitFunnel/Utilities/Factories.h"  // TODO: only for diagnosticStream. Remove.
#include "ByteCodeInterpreter.h"
#include "NativeCodeVerifier.h"
#include "CompileNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    static const ShardId c_shardId = 0;
    static const Term::StreamId c_streamId = 0;
    static const size_t c_allocatorBufferSize = 1000000;


    void TestXXX(ISimpleIndex const & index,
                 Rank initialRank)
    {
        NativeCodeVerifier v(index, initialRank);
    }


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
                                          size_t slice)
    {
        size_t bitPos = 63;

        while (accumulator != 0)
        {
            uint64_t count = lzcnt(accumulator);
            accumulator <<= count;
            bitPos -= count;

            DocIndex docIndex = offset * c_bitsPerQuadword + bitPos;
            DocumentHandle handle =
                Factories::CreateDocumentHandle(reinterpret_cast<Slice*>(slice), docIndex);

            accumulator <<= 1;
            --bitPos;
        }
    }


    void NativeCodeVerifier::Verify(char const * /*codeText*/)
    {
        // TODO: This check isn't too useful at the moment because
        // document 0 matches all queries. Therefore, even a poorly
        // constructed test will get at least one match.
        //if (m_expectNoResults)
        //{
        //    ASSERT_EQ(m_expectedResults.size(), 0ull)
        //        << "Expecting no results, but ExpectResult() was called"
        //        "at least once with a non-zero accumulator value.";
        //}
        //else
        //{
        //    ASSERT_GT(m_expectedResults.size(), 0ull)
        //        << "Expecting at least one result, but each call to "
        //        "ExpectResult() passed an empty accumulator.";
        //}

        //ByteCodeGenerator code;
        //GenerateCode(codeText, code);
        //code.Seal();

        //auto & shard = m_index.GetIngestor().GetShard(c_shardId);
        //auto & sliceBuffers = shard.GetSliceBuffers();
        //auto iterationsPerSlice = GetIterationsPerSlice();

        //// TODO: remove diagnosticStream and replace with nullable.
        //auto diagnosticStream = Factories::CreateDiagnosticStream(std::cout);
        //QueryInstrumentation instrumentation;
        //ByteCodeInterpreter interpreter(
        //    code,
        //    *this,
        //    sliceBuffers.size(),
        //    reinterpret_cast<char* const *>(sliceBuffers.data()),
        //    iterationsPerSlice,
        //    m_rowOffsets.data(),
        //    *diagnosticStream,
        //    instrumentation);

        //interpreter.Run();

        //// This check is necessary to detect the case where the final call
        //// to FinishIteration is missing.
        //EXPECT_EQ(m_resultsCount, m_expectedResults.size());
    }


    //
    // static methods
    //
}
