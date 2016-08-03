# BitFunnel/src/Common/Configuration/src

set(CPPFILES
    FileManager.cpp
)

set(WINDOWS_CPPFILES
    ParameterizedFile.cpp
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    FileManager.h
)

set(WINDOWS_PRIVATE_HFILES
    ParameterizedFile.h
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Configuration ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Configuration PROPERTY FOLDER "src/Common/Configuration")
set_property(TARGET Configuration PROPERTY PROJECT_LABEL "src")
