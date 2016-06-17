# BitFunnel/src/Index/src

set(CPPFILES
    ChunkEnumerator.cpp
    ChunkIngestor.cpp
    ChunkReader.cpp
    ChunkTaskProcessor.cpp
    Configuration.cpp
    Document.cpp
    DocumentHandleInternal.cpp
    DocumentLengthHistogram.cpp
    IngestChunks.cpp
    Ingestor.cpp
    Placeholder.cpp
    Shard.cpp
    Slice.cpp
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
    Document.h
    DocumentHandleInternal.h
    DocumentLengthHistogram.h
    Ingestor.h
    Shard.h
    Slice.h
    Term.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Index ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Index PROPERTY FOLDER "src/Index")
set_property(TARGET Index PROPERTY PROJECT_LABEL "src")
