#include "stdafx.h"

#include <fstream>

#include "BitFunnel/Factories.h"
#include "FileManager.h"
#include "LoggerInterfaces/Logging.h"
#include "ParameterizedFile.h"

namespace BitFunnel
{
    IFileManager* Factories::CreateFileManager(char const * configDirectory,
                                               char const * intermediateDirectory,
                                               char const * backupDirectory,
                                               char const * hddIndexDirectory,
                                               char const * ssdIndexDirectory)
    {
        return new FileManager(configDirectory,
                               intermediateDirectory,
                               backupDirectory,
                               hddIndexDirectory,
                               ssdIndexDirectory);
    }


    FileManager::FileManager(char const * configDirectory,
                             char const * intermediateDirectory,
                             char const * backupDirectory,
                             char const * hddIndexDirectory,
                             char const * /* ssdIndexDirectory */)
        : m_bandTable(new ParameterizedFile0(configDirectory, "BandTable", ".csv" )),
          m_commonNegatedTerms(new ParameterizedFile0(configDirectory, "CommonNegatedTerms", ".csv" )),
          m_commonPhrases(new ParameterizedFile0(configDirectory, "CommonPhrases", ".csv" )),
          m_docFreqTable(new ParameterizedFile0(intermediateDirectory, "DocumentFrequencyTable", ".bin" )),
          m_documentHistogram(new ParameterizedFile0(intermediateDirectory, "DocumentHistogram", ".csv" )),
          m_l1RankerConfig(new ParameterizedFile0(configDirectory, "L1BF4325", ".ini" )),
          m_manifest(new ParameterizedFile0(configDirectory, "ManifestList", ".csv" )),
          m_model(new ParameterizedFile0(configDirectory, "Model", ".bin" )),
          m_planDescriptors(new ParameterizedFile0(configDirectory, "PlanDescriptors", ".csv" )),
          m_postingCounts(new ParameterizedFile0(intermediateDirectory, "PostingCounts", ".csv" )),
          m_shardDefinition(new ParameterizedFile0(intermediateDirectory, "ShardDefinition", ".csv" )),
          m_shardDocCounts(new ParameterizedFile0(intermediateDirectory, "ShardDocCounts", ".csv" )),
          m_shardedDocFreqTable(new ParameterizedFile0(intermediateDirectory, "ShardedDocumentFrequencyTable", ".bin" )),
          m_sortRankerConfig(new ParameterizedFile0(configDirectory, "SN-20100315", ".ini" )),
          m_streamNameToSuffixMap(new ParameterizedFile0(configDirectory, "StreamNameToSuffix", ".csv" )),
          m_suffixToClassificationMap(new ParameterizedFile0(configDirectory, "SuffixToClassification", ".csv" )),
          m_clickStreamSuffixToMarketMap(new ParameterizedFile0(configDirectory, "ClickStreamSuffixToMarketMap", ".csv")),
          m_termTableStats(new ParameterizedFile0(intermediateDirectory, "TermTableStats", ".csv")),
          m_postingAndBitStats(new ParameterizedFile0(intermediateDirectory, "PostingAndBits", ".csv")),
          m_strengtheningMetawords(new ParameterizedFile0(configDirectory, "StrengtheningMetawords", ".csv")),
          m_docTable(new ParameterizedFile1<ShardId>(hddIndexDirectory, "DocTable", ".bin" )),
          m_scoreTable(new ParameterizedFile1<ShardId>(hddIndexDirectory, "ScoreTable", ".bin" )),
          m_termTable(new ParameterizedFile1<ShardId>(hddIndexDirectory, "TermTable", ".bin" )),
     
          m_tierDefinition(new ParameterizedFile0(configDirectory, "TierDefinition", ".csv" )),
          m_termDisposeDefinition(new ParameterizedFile0(configDirectory, "TermDisposeDefinition", ".csv" )),
          m_metaWordTierHintMap(new ParameterizedFile0(configDirectory, "MetaWordTierHintMap", ".csv" )),

          m_indexSlice(new ParameterizedFile2<ShardId, SliceId>(backupDirectory, "IndexSlice", ".bin"))
    {
    }


    FileDescriptor0 FileManager::BandTable()
    {
        return FileDescriptor0(*m_bandTable);
    }


    FileDescriptor0 FileManager::CommonNegatedTerms()
    {
        return FileDescriptor0(*m_commonNegatedTerms);
    }


    FileDescriptor0 FileManager::CommonPhrases()
    {
        return FileDescriptor0(*m_commonPhrases);
    }


    FileDescriptor0 FileManager::DocFreqTable()
    {
        return FileDescriptor0(*m_docFreqTable);
    }


    FileDescriptor0 FileManager::DocumentHistogram()
    {
        return FileDescriptor0(*m_documentHistogram);
    }


    FileDescriptor0 FileManager::L1RankerConfig()
    {
        return FileDescriptor0(*m_l1RankerConfig);
    }


    FileDescriptor0 FileManager::Manifest()
    {
        return FileDescriptor0(*m_manifest);
    }


    FileDescriptor0 FileManager::Model()
    {
        return FileDescriptor0(*m_model);
    }


    FileDescriptor0 FileManager::PlanDescriptors()
    {
        return FileDescriptor0(*m_planDescriptors);
    }


    FileDescriptor0 FileManager::PostingCounts()
    {
        return FileDescriptor0(*m_postingCounts);
    }


    FileDescriptor0 FileManager::ShardDefinition()
    {
        return FileDescriptor0(*m_shardDefinition);
    }


    FileDescriptor0 FileManager::ShardDocCounts()
    {
        return FileDescriptor0(*m_shardDocCounts);
    }


    FileDescriptor0 FileManager::ShardedDocFreqTable()
    {
        return FileDescriptor0(*m_shardedDocFreqTable);
    }


    FileDescriptor0 FileManager::SortRankerConfig()
    {
        return FileDescriptor0(*m_sortRankerConfig);
    }


    FileDescriptor0 FileManager::StreamNameToSuffixMap()
    {
        return FileDescriptor0(*m_streamNameToSuffixMap);
    }


    FileDescriptor0 FileManager::SuffixToClassificationMap()
    {
        return FileDescriptor0(*m_suffixToClassificationMap);
    }


    FileDescriptor0 FileManager::ClickStreamSuffixToMarketMap()
    {
        return FileDescriptor0(*m_clickStreamSuffixToMarketMap);
    }


    FileDescriptor0 FileManager::TierDefinition()
    {
        return FileDescriptor0(*m_tierDefinition);
    }


    FileDescriptor0 FileManager::TermDisposeDefinition()
    {
        return FileDescriptor0(*m_termDisposeDefinition);
    }


    FileDescriptor0 FileManager::MetaWordTierHintMap()
    {
        return FileDescriptor0(*m_metaWordTierHintMap);
    }


    FileDescriptor0 FileManager::TermTableStats()
    {
        return FileDescriptor0(*m_termTableStats);
    }


    FileDescriptor0 FileManager::PostingAndBitStats()
    {
        return FileDescriptor0(*m_postingAndBitStats);
    }


    FileDescriptor0 FileManager::StrengtheningMetawords()
    {
        return FileDescriptor0(*m_strengtheningMetawords);
    }


    FileDescriptor1<ShardId> FileManager::DocTable(ShardId shard)
    {
        return FileDescriptor1<ShardId>(*m_docTable, shard);
    }


    FileDescriptor1<ShardId> FileManager::ScoreTable(ShardId shard)
    {
        return FileDescriptor1<ShardId>(*m_scoreTable, shard);
    }


    FileDescriptor1<ShardId> FileManager::TermTable(ShardId shard)
    {
        return FileDescriptor1<ShardId>(*m_termTable, shard);
    }


    FileDescriptor2<ShardId, SliceId> FileManager::IndexSlice(ShardId shard, SliceId slice)
    {
        return FileDescriptor2<ShardId, SliceId>(*m_indexSlice, shard, slice);
    }
}
