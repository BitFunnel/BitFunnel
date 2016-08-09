# BitFunnel/src/Index/src

set(CPPFILES
    ChunkEnumerator.cpp
    ChunkIngestor.cpp
    ChunkReader.cpp
    ChunkTaskProcessor.cpp
    Configuration.cpp
    DocTableDescriptor.cpp
    Document.cpp
    DocumentDataSchema.cpp
    DocumentFrequencyTable.cpp
    DocumentFrequencyTableBuilder.cpp
    DocumentHandleInternal.cpp
    DocumentLengthHistogram.cpp
    DocumentMap.cpp
    FactSetBase.cpp
    IngestChunks.cpp
    Ingestor.cpp
    PackedTermInfo.cpp
    Recycler.cpp
    RowId.cpp
    RowConfiguration.cpp
    RowTableDescriptor.cpp
    Shard.cpp
    Slice.cpp
    SliceBufferAllocator.cpp
    Term.cpp
    TermInfo.cpp
    TermTable.cpp
    TermTableBuilder.cpp
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
    DocumentDataSchema.h
    DocumentFrequencyTable.h
    DocumentFrequencyTableBuilder.h
    DocumentHandleInternal.h
    DocumentLengthHistogram.h
    DocumentMap.h
    FactSetBase.h
    Ingestor.h
    IRecyclable.h
    IRecycler.h
    ISliceBufferAllocator.h
    Recycler.h
    RowTableDescriptor.h
    Shard.h
    Slice.h
    SliceBufferAllocator.h
    TermTable.h
    TermTableBuilder.h
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
