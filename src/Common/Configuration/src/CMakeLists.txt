# BitFunnel/src/Common/Configuration/src

set(CPPFILES
    FileManager.cpp
    ParameterizedFile.cpp
    ShardDefinition.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    FileManager.h
    ParameterizedFile.h
    ShardDefinition.cpp
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Configuration ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Configuration PROPERTY FOLDER "src/Common/Configuration")
set_property(TARGET Configuration PROPERTY PROJECT_LABEL "src")
