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
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Mocks/Factories.h"
#include "BitFunnel/Plan/IResultsProcessor.h"
#include "BitFunnel/Term.h"
#include "ByteCodeInterpreter.h"
#include "CompileNode.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    class ResultsProcessor : public IResultsProcessor
    {
    public:
        void AddResult(uint64_t accumulator,
                       size_t offset) override
        {
            std::cout
                << "AddResult("
                << std::hex << accumulator
                << ", " << offset
                << ")" << std::endl;
        }


        bool FinishIteration(void const * /*sliceBuffer*/) override
        {
            std::cout
                << "FinishIteration()" << std::endl;
            return false;
        }


        bool TerminatedEarly() const override
        {
            std::cout
                << "TerminatedEarly()" << std::endl;
            return false;
        }

    private:
    };

    size_t c_allocatorBufferSize = 1000000;

    void GenerateCode(char const * rowPlanText,
                      ByteCodeGenerator& code)
    {
        std::stringstream rowPlan(rowPlanText);

        Allocator allocator(c_allocatorBufferSize);
        TextObjectParser parser(rowPlan, allocator, &CompileNode::GetType);

        CompileNode const & node = CompileNode::Parse(parser);

        node.Compile(code);
    }


    RowId GetFirstRow(ITermTable const & termTable,
                      Term term)
    {
        RowIdSequence rows(term, termTable);

        auto it = rows.begin();
        // TODO: Implement operator << for RowIdSequence::const_iterator.
        //CHECK_NE(it, rows.end())
        //    << "Expected at least one row.";

        RowId row =  *it;

        ++it;
        // TODO: Implement operator << for RowIdSequence::const_iterator.
        //CHECK_EQ(it, rows.end())
        //    << "Expected no more than one row.";

        return row;
    }


    ptrdiff_t GetRowOffset(char const * text,
                           Term::StreamId stream,
                           IConfiguration const & config,
                           ITermTable const & termTable,
                           IShard const & shard)
    {
        Term term(text, stream, config);
        RowId row = GetFirstRow(termTable, term);
        return shard.GetRowOffset(row);
    }


    void RunTest(ByteCodeGenerator const & code)
    {
        const DocId maxDocId = 800;
        const Term::StreamId streamId = 0;

        const size_t maxGramSize = 1;

        auto fileSystem = Factories::CreateRAMFileSystem();

        auto index =
            Factories::CreatePrimeFactorsIndex(*fileSystem,
                                               maxDocId,
                                               streamId);

        const ShardId shardId = 0;
        auto & shard = index->GetIngestor().GetShard(shardId);

        std::vector<ptrdiff_t> rowOffsets;

        rowOffsets.push_back(GetRowOffset(
            "0",
            streamId,
            index->GetConfiguration(),
            index->GetTermTable(),
            shard));
        rowOffsets.push_back(GetRowOffset(
            "1",
            streamId,
            index->GetConfiguration(),
            index->GetTermTable(),
            shard));
        rowOffsets.push_back(GetRowOffset(
            "2",
            streamId,
            index->GetConfiguration(),
            index->GetTermTable(),
            shard));

        Rank c_maxRank = 0;

        auto & sliceBuffers = shard.GetSliceBuffers();
        auto iterationsPerSlice = shard.GetSliceCapacity() / (64ull << c_maxRank);

        ResultsProcessor resultsProcessor;

        ByteCodeInterpreter interpreter(
            code,
            resultsProcessor,
            sliceBuffers.size(),
            reinterpret_cast<char* const *>(sliceBuffers.data()),
            iterationsPerSlice,
            rowOffsets.data());

        interpreter.Run();
    }


    //class MockSlice
    //{
    //public:
    //    MockSlice(size_t sliceNumber,
    //              size_t quadwordsPerSlice,
    //              size_t rowCount);

    //    std::vector<ptrdiff_t>
    //        GetRowOffsets(std::vector<size_t> rowIndices) const;

    //private:
    //    std::vector<std::vector<uint64_t>> m_rows;
    //};

    //MockSlice::MockSlice(size_t sliceNumber,
    //                     size_t quadwordsPerSlice,
    //                     size_t rowCount)
    //{

    //}

    //class MockIndex
    //{
    //public:
    //    MockIndex(size_t sliceCount,
    //              size_t quadwordsPerSlice,
    //              size_t rowCount);

    //private:
    //    std::vector<MockSlice> m_slices;
    //};


    //void RunTest2()
    //{
    //    std::vector<uint64_t> rowOffsetsSlice1 =
    //}


    TEST(ByteCodeInterpreter, Placeholder)
    {
        char const * text =
            "LoadRowJz {"
            "  Row: Row(0, 0, 0, false),"      // Row(0) is 0, 1, 2, ...
            "  Child: AndRowJz {"
            "    Row: Row(2, 0, 0, false),"    // Row(2) is AAAAAAA....
            "    Child: Report {"
            "      Child: "
            "    }"
            "  }"
            "}";

        ByteCodeGenerator code;
        GenerateCode(text, code);
        code.Seal();

        RunTest(code);
    }
}
