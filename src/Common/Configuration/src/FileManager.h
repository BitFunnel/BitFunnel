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

#include <memory>           // For std::unique_ptr.

#include "BitFunnel/IFileManager.h"


namespace BitFunnel
{
    class IFileSystem;

    class FileManager : public IFileManager
    {
    public:
        FileManager(char const * configDirectory,
                    char const * statisticsDirectory,
                    char const * indexDirectory,
                    IFileSystem & fileSystem);

        //
        // IFileManager methods.
        //
        //virtual FileDescriptor0 BandTable() override;
        //virtual FileDescriptor0 ClickStreamSuffixToMarketMap() override;
        //virtual FileDescriptor0 CommonNegatedTerms() override;
        //virtual FileDescriptor0 CommonPhrases() override;
        //virtual FileDescriptor0 DocFreqTable() override;
        virtual FileDescriptor0 ColumnDensitySummary() override;
        virtual FileDescriptor0 DocumentHistogram() override;
        //virtual FileDescriptor0 L1RankerConfig() override;
        virtual FileDescriptor0 Manifest() override;
        //virtual FileDescriptor0 Model() override;
        //virtual FileDescriptor0 PlanDescriptors() override;
        //virtual FileDescriptor0 PostingCounts() override;
        virtual FileDescriptor0 QueryLog() override;
        virtual FileDescriptor0 QueryPipelineStatistics() override;
        virtual FileDescriptor0 QuerySummaryStatistics() override;
        virtual FileDescriptor0 RowDensitySummary() override;
        virtual FileDescriptor0 ShardDefinition() override;
        //virtual FileDescriptor0 ShardDocCounts() override;
        //virtual FileDescriptor0 ShardedDocFreqTable() override;
        //virtual FileDescriptor0 SortRankerConfig() override;
        //virtual FileDescriptor0 StreamNameToSuffixMap() override;
        //virtual FileDescriptor0 SuffixToClassificationMap() override;
        virtual FileDescriptor0 TermToText() override;
        //virtual FileDescriptor0 TierDefinition() override;
        //virtual FileDescriptor0 TermDisposeDefinition() override;
        //virtual FileDescriptor0 MetaWordTierHintMap() override;
        //virtual FileDescriptor0 TermTableStats() override;
        //virtual FileDescriptor0 PostingAndBitStats() override;
        //virtual FileDescriptor0 StrengtheningMetawords() override;
        virtual FileDescriptor0 VerificationResults() override;

        virtual FileDescriptor1 ColumnDensities(size_t shard) override;
        virtual FileDescriptor1 Chunk(size_t number) override;
        virtual FileDescriptor1 Correlate(size_t shard) override;
        virtual FileDescriptor1 CumulativeTermCounts(size_t shard) override;
        virtual FileDescriptor1 DocFreqTable(size_t shard) override;
        //virtual FileDescriptor1 DocTable(size_t shard) override;
        //virtual FileDescriptor1 ScoreTable(size_t shard) override;
        virtual FileDescriptor1 RowDensities(size_t shard) override;
        virtual FileDescriptor1 TermTable(size_t shard) override;
        virtual FileDescriptor1 TermTableStatistics(size_t shard) override;

        virtual FileDescriptor0 IndexSliceMain() override;
        virtual FileDescriptor2 IndexSlice(size_t shard, size_t slice) override;

    private:
        std::unique_ptr<IParameterizedFile1> m_chunk;
        std::unique_ptr<IParameterizedFile1> m_columnDensities;
        std::unique_ptr<IParameterizedFile0> m_columnDensitySummary;
        std::unique_ptr<IParameterizedFile1> m_correlate;
        std::unique_ptr<IParameterizedFile1> m_cumulativeTermCounts;
        std::unique_ptr<IParameterizedFile1> m_docFreqTable;
        std::unique_ptr<IParameterizedFile0> m_documentHistogram;
        std::unique_ptr<IParameterizedFile0> m_indexSliceMain;
        std::unique_ptr<IParameterizedFile2> m_indexSlice;
        std::unique_ptr<IParameterizedFile0> m_manifest;
        std::unique_ptr<IParameterizedFile0> m_queryLog;
        std::unique_ptr<IParameterizedFile0> m_queryPipelineStatistics;
        std::unique_ptr<IParameterizedFile0> m_querySummaryStatistics;
        std::unique_ptr<IParameterizedFile1> m_rowDensities;
        std::unique_ptr<IParameterizedFile0> m_rowDensitySummary;
        std::unique_ptr<IParameterizedFile0> m_shardDefinition;
        std::unique_ptr<IParameterizedFile1> m_termTable;
        std::unique_ptr<IParameterizedFile1> m_termTableStatistics;
        std::unique_ptr<IParameterizedFile0> m_termToText;
        std::unique_ptr<IParameterizedFile0> m_verificationResults;
    };
}
