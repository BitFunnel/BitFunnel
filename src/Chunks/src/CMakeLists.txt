# BitFunnel/src/Chunks/src

set(CPPFILES
    BuiltinChunkManifest.cpp
    ChunkEnumerator.cpp
    ChunkIngestor.cpp
    ChunkManifestIngestor.cpp
    ChunkReader.cpp
	ChunkWriters.cpp
    Document.cpp
    DocumentFilters.cpp
    IngestChunks.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    BuiltinChunkManifest.h
    ChunkEnumerator.h
    ChunkIngestor.h
    ChunkManifestIngestor.h
    ChunkReader.h
	ChunkWriters.h
    Document.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Chunks ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Chunks PROPERTY FOLDER "src/Chunks")
set_property(TARGET Chunks PROPERTY PROJECT_LABEL "src")
