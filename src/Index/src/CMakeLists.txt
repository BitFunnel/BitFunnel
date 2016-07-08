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
    DocumentHandleInternal.cpp
    DocumentLengthHistogram.cpp
    IngestChunks.cpp
    Ingestor.cpp
    PackedTermInfo.cpp
    Placeholder.cpp
    Recycler.cpp
    RowId.cpp
    RowTableDescriptor.cpp
    Shard.cpp
    Slice.cpp
    SliceBufferAllocator.cpp
    Term.cpp
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
    DocumentHandleInternal.h
    DocumentLengthHistogram.h
    Ingestor.h
    IRecyclable.h
    IRecycler.h
    ISliceBufferAllocator.h
    Recycler.h
    RowTableDescriptor.h
    Shard.h
    Slice.h
    SliceBufferAllocator.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Index ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Index PROPERTY FOLDER "src/Index")
set_property(TARGET Index PROPERTY PROJECT_LABEL "src")
