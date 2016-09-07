# BitFunnel/src/Index/src

set(CPPFILES
    ChunkEnumerator.cpp
    ChunkIngestor.cpp
    ChunkReader.cpp
    ChunkTaskProcessor.cpp
    Configuration.cpp
    DocTableDescriptor.cpp
    Document.cpp
    DocumentCache.cpp
    DocumentDataSchema.cpp
    DocumentFrequencyTable.cpp
    DocumentFrequencyTableBuilder.cpp
    DocumentHandleInternal.cpp
    DocumentLengthHistogram.cpp
    DocumentMap.cpp
    FactSetBase.cpp
    Helpers.cpp
    IDocumentCache.cpp
    IndexedIdfTable.cpp
    IngestChunks.cpp
    Ingestor.cpp
    PackedRowIdSequence.cpp
    Recycler.cpp
    RowId.cpp
    RowIdSequence.cpp
    RowConfiguration.cpp
    RowTableDescriptor.cpp
    Shard.cpp
    SimpleIndex.cpp
    Slice.cpp
    SliceBufferAllocator.cpp
    Term.cpp
    TermTable.cpp
    TermTableBuilder.cpp
    TermTableCollection.cpp
    TermToText.cpp
    TermTreatments.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    ChunkEnumerator.h
    ChunkIngestor.h
    ChunkReader.h
    ChunkTaskProcessor.h
    Configuration.h
    DocTableDescriptor.h
    Document.h
    DocumentCache.h
    DocumentDataSchema.h
    DocumentFrequencyTable.h
    DocumentFrequencyTableBuilder.h
    DocumentHandleInternal.h
    DocumentLengthHistogram.h
    DocumentMap.h
    FactSetBase.h
    IDocumentCacheNode.h
    IndexedIdfTable.h
    Ingestor.h
    IRecyclable.h
    Recycler.h
    RowTableDescriptor.h
    Shard.h
    SimpleIndex.h
    Slice.h
    SliceBufferAllocator.h
    TermTable.h
    TermTableBuilder.h
    TermTableCollection.h
    TermTreatments.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Index ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Index PROPERTY FOLDER "src/Index")
set_property(TARGET Index PROPERTY PROJECT_LABEL "src")
