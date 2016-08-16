# BitFunnel/src/Plan/src

set(CPPFILES
    CompileNodes.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    CompileNodes.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Plan ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Plan PROPERTY FOLDER "src/Plan")
set_property(TARGET Plan PROPERTY PROJECT_LABEL "src")
