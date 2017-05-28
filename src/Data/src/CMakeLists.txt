# BitFunnel/src/Data/src

set(CPPFILES
    SimpleData.cpp
    Sequential.cpp
    Sonnets.cpp
    SyntheticChunks.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Data ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Data PROPERTY FOLDER "src/Data")
set_property(TARGET Data PROPERTY PROJECT_LABEL "src")
