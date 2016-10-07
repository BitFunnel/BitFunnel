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
#include "ByteCodeVerifier.h"
#include "Primes.h"


namespace BitFunnel
{
    // TODO: This constant is in ByteCodeVerifier as well.
    static const Term::StreamId c_streamId = 0;


    static std::unique_ptr<IFileSystem> g_fileSystem;
    static std::unique_ptr<ISimpleIndex> g_index;
    static const DocId c_maxDocId = 832;


    //
    // Returns the ISimpleIndex that is shared by all of the tests
    // in this file.
    //
    ISimpleIndex const & GetIndex()
    {
        // Create the IFileSystem on first call.
        if (g_fileSystem.get() == nullptr)
        {
            g_fileSystem = Factories::CreateRAMFileSystem();
        }

        // Create the ISimpleIndex on first call.
        if (g_index.get() == nullptr)
        {
            g_index = Factories::CreatePrimeFactorsIndex(*g_fileSystem,
                                                         c_maxDocId,
                                                         c_streamId);
        }

        return *g_index;
    }


    //*************************************************************************
    //
    // AndRowJz test cases
    //
    //*************************************************************************
    TEST(ByteCodeInterpreter, AndRowJzDelta0)
    {
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
        ByteCodeVerifier verifier(GetIndex(), initialRank);

        verifier.DeclareRow("2");
        verifier.DeclareRow("3");

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


    TEST(ByteCodeInterpreter, AndRowJzDelta0Inverted)
    {
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
        ByteCodeVerifier verifier(GetIndex(), initialRank);

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


    TEST(ByteCodeInterpreter, AndRowJzDelta1)
    {
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
        ByteCodeVerifier verifier(GetIndex(), initialRank);

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


    TEST(ByteCodeInterpreter, AndRowJzDelta1Inverted)
    {
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
        ByteCodeVerifier verifier(GetIndex(), initialRank);

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
            const uint64_t row0 = verifier.GetRowData(0,  offset / 2, slice);
            verifier.ExpectResult(row1 & ~row0, offset, slice);
        }

        verifier.Verify(text);
    }


    TEST(ByteCodeInterpreter, AndRowJzMatches)
    {
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
        ByteCodeVerifier verifier(GetIndex(), initialRank);
        verifier.VerboseMode(true);

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
    // RankDown test cases
    //
    //*************************************************************************
    TEST(ByteCodeInterpreter, RankDownDelta1)
    {
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
        ByteCodeVerifier verifier(GetIndex(), initialRank);

        verifier.VerboseMode(true);

        verifier.DeclareRow("3");

        for (auto iteration : verifier.GetIterations())
        {
            const size_t slice = verifier.GetSliceNumber(iteration);
            const size_t offset = verifier.GetOffset(iteration);

            for (size_t i = 0; i < 2; ++i)
            {
                const uint64_t row0 = verifier.GetRowData(0, offset + i, slice);
                verifier.ExpectResult(row0, offset + i, slice);
            }
        }

        verifier.Verify(text);
    }
}
