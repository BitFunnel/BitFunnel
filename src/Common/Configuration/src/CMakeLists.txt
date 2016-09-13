# BitFunnel/src/Common/Configuration/src

set(CPPFILES
    FileManager.cpp
    ParameterizedFile.cpp
    RAMFileSystem.cpp
    ShardDefinition.cpp
    StreamConfiguration.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    FileManager.h
    ParameterizedFile.h
    RAMFileSystem.h
    ShardDefinition.h
    StreamConfiguration.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Configuration ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Configuration PROPERTY FOLDER "src/Common/Configuration")
set_property(TARGET Configuration PROPERTY PROJECT_LABEL "src")
