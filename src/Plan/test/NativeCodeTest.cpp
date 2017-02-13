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
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Mocks/Factories.h"
#include "BitFunnel/Term.h"                     // Only needed for streamId
#include "NativeCodeVerifier.h"
#include "Primes.h"


namespace BitFunnel
{
    // TODO: This constant is in ByteCodeVerifier as well.
    // static const Term::StreamId c_streamId = 0;


    static std::unique_ptr<IFileSystem> g_fileSystem;
    static std::unique_ptr<ISimpleIndex> g_index;

    // Need enough documents to get iterations at maximum rank used in a test
    // which is rank 2. 1664 / (64 documents per quadword) / (4 for rank2) gives
    // 6.5 quadwords at rank 8.
    // static const DocId c_maxDocId = 1664;

    // TODO: This should come from a shared header.
    extern ISimpleIndex const & GetIndex(ShardId numShards);


    //*************************************************************************
    //
    // AndRowJz test cases
    //
    //*************************************************************************
    TEST(NativeCode, AndRowJzDelta0)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(1, 0, 0, false),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";


        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("5");
        verifier.DeclareRow("7");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            verifier.ExpectResult(row0 & row1, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(NativeCode, AndRowJzDelta0Shards2)
    {
        ShardId c_numShards = 2;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(1, 0, 0, false),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";


        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("5");
        verifier.DeclareRow("7");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            verifier.ExpectResult(row0 & row1, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(NativeCode, AndRowJzDelta0Inverted)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(1, 0, 0, true),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("2");
        verifier.DeclareRow("3");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            verifier.ExpectResult(row0 & ~row1, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(NativeCode, AndRowJzDelta1)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(1, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(0, 0, 1, false),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        // IMPORTANT: row0 must be a row whose value differs
        // across adjacent quadwords in order to correctly
        // test the rank delta. If row0 has the same value in
        // adjacent quadwords,
        //     GetRowData(0, slice, offset)
        // will return the same value as
        //     GetRowData(0, slice, offset /2);
        verifier.DeclareRow("3");
        verifier.DeclareRow("2");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            const uint64_t row0 = verifier.GetRowData(0, offset / 2, slice);
            verifier.ExpectResult(row1 & row0, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(NativeCode, AndRowJzDelta1Inverted)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(1, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(0, 0, 1, true),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        // IMPORTANT: row0 must be a row whose value differs
        // across adjacent quadwords in order to correctly
        // test the rank delta. If row0 has the same value in
        // adjacent quadwords,
        //     GetRowData(0, slice, offset)
        // will return the same value as
        //     GetRowData(0, slice, offset /2);
        verifier.DeclareRow("3");
        verifier.DeclareRow("2");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            const uint64_t row0 = verifier.GetRowData(0, offset / 2, slice);
            verifier.ExpectResult(row1 & ~row0, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(NativeCode, AndRowJzMatches)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: AndRowJz {"
            "    Row: Row(1, 0, 0, false),"
            "    Child: AndRowJz {"
            "      Row: Row(2, 0, 0, false),"
            "      Child: Report {"
            "        Child: "
            "      }"
            "    }"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("2");
        verifier.DeclareRow("3");
        verifier.DeclareRow("5");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            const uint64_t row2 = verifier.GetRowData(2, offset, slice);
            verifier.ExpectResult(row0 & row1 & row2, offset, slice);
        }

        verifier.Verify(text);
    }


    //*************************************************************************
    //
    // LoadRowJz test cases
    //
    //*************************************************************************

    //
    // LoadRowJz, Rank = 0, RankDelta = 0
    //
    TEST(NativeCode, LoadRowJzRank0Delta0)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: Report {"
            "    Child: "
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("3");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            verifier.ExpectResult(row0, offset, slice);
        }

        verifier.Verify(text);
    }



    //
    // LoadRowJz, Rank = 6, RankDelta = 0
    // (no equivalent test possible)
    //


    //
    // LoadRowJz, Rank = 0, RankDelta = 0, inverted
    //
    TEST(NativeCode, LoadRowJzRank0Delta0Inverted)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, true),"
            "  Child: Report {"
            "    Child: "
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("5");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            verifier.ExpectResult(~row0, offset, slice);
        }

        verifier.Verify(text);
    }


    //
    // LoadRowJz, Rank = 0, RankDelta = 1
    //
    TEST(NativeCode, LoadRowJzRank0Delta1)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 1, false),"
            "  Child: Report {"
            "    Child: "
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("7");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset >> 1, slice);
            verifier.ExpectResult(row0, offset, slice);
        }

        verifier.Verify(text);
    }


    //
    // LoadRowJz, Rank = 0, RankDelta = 1, inverted
    //
    TEST(NativeCode, LoadRowJzRank0Delta1Inverted)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 1, true),"
            "  Child: Report {"
            "    Child: "
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("11");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset >> 1, slice);
            verifier.ExpectResult(~row0, offset, slice);
        }

        verifier.Verify(text);
    }


    //*************************************************************************
    //
    // LoadRow test cases
    //
    //*************************************************************************

    //
    // LoadRow, RankDelta = 0
    //
    TEST(NativeCode, LoadRowDelta0)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: Report {"
            "    Child: LoadRow(1, 0, 0, false)"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("3");
        verifier.DeclareRow("5");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            verifier.ExpectResult(row0 & row1, offset, slice);
        }

        verifier.Verify(text);
    }


    //
    // LoadRow, RankDelta = 0, inverted
    //
    TEST(NativeCode, LoadRowDelta0Inverted)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: Report {"
            "    Child: LoadRow(1, 0, 0, true)"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("3");
        verifier.DeclareRow("5");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            verifier.ExpectResult(row0 & ~row1, offset, slice);
        }

        verifier.Verify(text);
    }


    //
    // LoadRow, RankDelta = 1
    //
    TEST(NativeCode, LoadRowDelta1)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: Report {"
            "    Child: LoadRow(1, 0, 1, false)"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("3");
        verifier.DeclareRow("5");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset >> 1, slice);
            verifier.ExpectResult(row0 & row1, offset, slice);
        }

        verifier.Verify(text);
    }


    //
    // LoadRow, RankDelta = 1, inverted
    //
    TEST(NativeCode, LoadRowDelta1Inverted)
    {
        ShardId c_numShards = 1;
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"
            "  Child: Report {"
            "    Child: LoadRow(1, 0, 1, true)"
            "  }"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("3");
        verifier.DeclareRow("5");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset >> 1, slice);
            verifier.ExpectResult(row0 & ~row1, offset, slice);
        }

        verifier.Verify(text);
    }


    //*************************************************************************
    //
    // Or test cases
    //
    //*************************************************************************

    TEST(NativeCode, OrMatches)
    {
        ShardId c_numShards = 1;
        char const * text =
            "Or {"
            "  Children: ["
            "    LoadRowJz {"
            "      Row: Row(0, 0, 0, false),"
            "      Child: Report {"
            "        Child: "
            "      }"
            "    },"
            "    LoadRowJz {"
            "      Row: Row(1, 0, 0, false),"
            "      Child: Report {"
            "        Child: "
            "      }"
            "    }"
            "  ]"
            "}";

        const Rank initialRank = 0;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("3");
        verifier.DeclareRow("5");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            const uint64_t row0 = verifier.GetRowData(0, offset, slice);
            const uint64_t row1 = verifier.GetRowData(1, offset, slice);
            verifier.ExpectResult(row0, offset, slice);
            verifier.ExpectResult(row1, offset, slice);
        }

        verifier.Verify(text);
    }


    //*************************************************************************
    //
    // Out-of-order test cases
    //
    //*************************************************************************

    // TODO: Verify that this test is actually working correctly and not just
    // spuriously passing. Have no reason to suspect test is wrong. It just
    // seems complex.
    TEST(NativeCode, OutOfOrderMatches)
    {
        ShardId c_numShards = 1;
        char const * text =
            "RankDown {"
            "  Delta: 2,"
            "  Child: LoadRowJz {"
            "    Row: Row(0, 0, 0, false),"
            "    Child: AndRowJz {"
            "      Row: Row(1, 0, 1, false),"
            "      Child: AndRowJz {"
            "        Row: Row(2, 0, 2, false),"
            "        Child: Report {"
            "          Child: "
            "        }"
            "      }"
            "    }"
            "  }"
            "}";


        const Rank initialRank = 2;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.DeclareRow("3");
        verifier.DeclareRow("5");
        verifier.DeclareRow("7");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            for (size_t i = 0; i < 4; ++i)
            {
                const uint64_t row0 = verifier.GetRowData(0, offset + i, slice);
                const uint64_t row1 = verifier.GetRowData(1, (offset + i) >> 1, slice);
                const uint64_t row2 = verifier.GetRowData(2, (offset + i) >> 2, slice);

                verifier.ExpectResult(row0 & row1 & row2, offset + i, slice);
            }
        }

        verifier.Verify(text);
    }


    //*************************************************************************
    //
    // RankDown test cases
    //
    //*************************************************************************
    TEST(NativeCode, RankDownDelta1)
    {
        ShardId c_numShards = 1;
        char const * text =
            "RankDown {"
            "  Delta: 1,"
            "  Child: LoadRowJz {"
            "    Row: Row(0, 5, 0, false),"
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        const Rank initialRank = 1;
        NativeCodeVerifier verifier(GetIndex(c_numShards), initialRank);

        verifier.VerboseMode(true);

        verifier.DeclareRow("3");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            for (size_t i = 0; i < 2; ++i)
            {
                const uint64_t row0 = verifier.GetRowData(0, offset * 2 + i, slice);
                verifier.ExpectResult(row0, offset * 2+ i, slice);
            }
        }

        verifier.Verify(text);
    }
}
