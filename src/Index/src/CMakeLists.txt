# BitFunnel/src/Index/src

set(CPPFILES
    Configuration.cpp
    Correlate.cpp
    DocTableDescriptor.cpp
    DocumentCache.cpp
    DocumentDataSchema.cpp
    DocumentFrequencyTable.cpp
    DocumentFrequencyTableBuilder.cpp
    DocumentHandleInternal.cpp
    DocumentHistogram.cpp
    DocumentHistogramBuilder.cpp
    DocumentMap.cpp
    FactSetBase.cpp
    Helpers.cpp
    IDocumentCache.cpp
    Ingestor.cpp
    PackedRowIdSequence.cpp
    Recycler.cpp
    RowId.cpp
    RowIdSequence.cpp
    RowConfiguration.cpp
    RowTableAnalyzer.cpp
    RowTableDescriptor.cpp
    Shard.cpp
    ShardDefinitionBuilder.cpp
    ShardCostFunction.cpp
    SimpleIndex.cpp
    SingleSourceShortestPath.cpp
    Slice.cpp
    SliceBufferAllocator.cpp
    Term.cpp
    TermTable.cpp
    TermTableBuilder.cpp
    TermTableCollection.cpp
    TermToText.cpp
    TermTreatmentFactory.cpp
    TreatmentClassicBitsliced.cpp
    TreatmentOptimal.cpp
    TreatmentPrivateRank0.cpp
    TreatmentPrivateSharedRank0.cpp
    TreatmentPrivateSharedRank0And3.cpp
    TreatmentPrivateSharedRank0ToN.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    Configuration.h
    Correlate.h
    DocTableDescriptor.h
    DocumentCache.h
    DocumentDataSchema.h
    DocumentFrequencyTable.h
    DocumentFrequencyTableBuilder.h
    DocumentHandleInternal.h
    DocumentHistogram.h
    DocumentHistogramBuilder.h
    DocumentMap.h
    FactSetBase.h
    IDocumentCacheNode.h
    Ingestor.h
    IRecyclable.h
    Recycler.h
    RowTableDescriptor.h
    RowTableAnalyzer.h
    Shard.h
    ShardCostFunction.h
    SimpleIndex.h
    SingleSourceShortestPath.h
    Slice.h
    SliceBufferAllocator.h
    TermTable.h
    TermTableBuilder.h
    TermTableCollection.h
    TermTreatmentFactory.h
    TreatmentClassicBitsliced.h
    TreatmentOptimal.h
    TreatmentPrivateRank0.h
    TreatmentPrivateSharedRank0.h
    TreatmentPrivateSharedRank0And3.h
    TreatmentPrivateSharedRank0ToN.cpp
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Index ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Index PROPERTY FOLDER "src/Index")
set_property(TARGET Index PROPERTY PROJECT_LABEL "src")
