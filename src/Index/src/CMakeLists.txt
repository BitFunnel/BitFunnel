# BitFunnel/src/Index/src

set(CPPFILES
    ChunkEnumerator.cpp
    ChunkIngestor.cpp
    ChunkReader.cpp
    ChunkTaskProcessor.cpp
    Document.cpp
    IngestChunks.cpp
    Placeholder.cpp
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
    Document.h
    DocumentLengthHistogram.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Index ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Index PROPERTY FOLDER "src/Index")
set_property(TARGET Index PROPERTY PROJECT_LABEL "src")
