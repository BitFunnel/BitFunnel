#pragma once

#include <memory>           // For std::unique_ptr.

#include "BitFunnel/IFileManager.h"

namespace BitFunnel
{
    class FileManager : public IFileManager
    {
    public:
        FileManager(char const * configDirectory,
                    char const * intermediateDirectory,
                    char const * backupDirectory,
                    char const * hddIndexDirectory,
                    char const * ssdIndexDirectory);

        //
        // IFileManager methods.
        //
        virtual FileDescriptor0 BandTable() override;
        virtual FileDescriptor0 CommonNegatedTerms() override;
        virtual FileDescriptor0 CommonPhrases() override;
        virtual FileDescriptor0 DocFreqTable() override;
        virtual FileDescriptor0 DocumentHistogram() override;
        virtual FileDescriptor0 L1RankerConfig() override;
        virtual FileDescriptor0 Manifest() override;
        virtual FileDescriptor0 Model() override;
        virtual FileDescriptor0 PlanDescriptors() override;
        virtual FileDescriptor0 PostingCounts() override;
        virtual FileDescriptor0 ShardDefinition() override;
        virtual FileDescriptor0 ShardDocCounts() override;
        virtual FileDescriptor0 ShardedDocFreqTable() override;
        virtual FileDescriptor0 SortRankerConfig() override;
        virtual FileDescriptor0 StreamNameToSuffixMap() override;
        virtual FileDescriptor0 SuffixToClassificationMap() override;
        virtual FileDescriptor0 ClickStreamSuffixToMarketMap() override;
        virtual FileDescriptor0 TierDefinition() override;
        virtual FileDescriptor0 TermDisposeDefinition() override;
        virtual FileDescriptor0 MetaWordTierHintMap() override;
        virtual FileDescriptor0 TermTableStats() override;
        virtual FileDescriptor0 PostingAndBitStats() override;
        virtual FileDescriptor0 StrengtheningMetawords() override;

        virtual FileDescriptor1<ShardId> DocTable(ShardId shard) override;
        virtual FileDescriptor1<ShardId> ScoreTable(ShardId shard) override;
        virtual FileDescriptor1<ShardId> TermTable(ShardId shard) override;

        virtual FileDescriptor2<ShardId, SliceId> IndexSlice(ShardId shard, SliceId slice) override;

    private:
        std::unique_ptr<IParameterizedFile0> m_bandTable;
        std::unique_ptr<IParameterizedFile0> m_commonNegatedTerms;
        std::unique_ptr<IParameterizedFile0> m_commonPhrases;
        std::unique_ptr<IParameterizedFile0> m_docFreqTable;
        std::unique_ptr<IParameterizedFile0> m_documentHistogram;
        std::unique_ptr<IParameterizedFile0> m_l1RankerConfig;
        std::unique_ptr<IParameterizedFile0> m_manifest;
        std::unique_ptr<IParameterizedFile0> m_model;
        std::unique_ptr<IParameterizedFile0> m_planDescriptors;
        std::unique_ptr<IParameterizedFile0> m_postingCounts;
        std::unique_ptr<IParameterizedFile0> m_shardCapacity;
        std::unique_ptr<IParameterizedFile0> m_shardDefinition;
        std::unique_ptr<IParameterizedFile0> m_shardDocCounts;
        std::unique_ptr<IParameterizedFile0> m_shardedDocFreqTable;
        std::unique_ptr<IParameterizedFile0> m_sortRankerConfig;
        std::unique_ptr<IParameterizedFile0> m_streamNameToSuffixMap;
        std::unique_ptr<IParameterizedFile0> m_suffixToClassificationMap;
        std::unique_ptr<IParameterizedFile0> m_clickStreamSuffixToMarketMap;
        std::unique_ptr<IParameterizedFile0> m_tierDefinition;
        std::unique_ptr<IParameterizedFile0> m_termDisposeDefinition;
        std::unique_ptr<IParameterizedFile0> m_metaWordTierHintMap;
        std::unique_ptr<IParameterizedFile0> m_termTableStats;
        std::unique_ptr<IParameterizedFile0> m_postingAndBitStats;
        std::unique_ptr<IParameterizedFile0> m_strengtheningMetawords;

        // TODO: cleanup unused files, e.g. DocTable, ScoreTable, RowTableSlice etc.
        std::unique_ptr<IParameterizedFile1<ShardId>> m_docTable;
        std::unique_ptr<IParameterizedFile1<ShardId>> m_scoreTable;
        std::unique_ptr<IParameterizedFile1<ShardId>> m_termTable;

        std::unique_ptr<IParameterizedFile2<ShardId, SliceId>> m_indexSlice;
    };
}
