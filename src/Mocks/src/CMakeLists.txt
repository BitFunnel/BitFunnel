# BitFunnel/src/Mocks/src

set(CPPFILES
    PrimeFactorsDocument.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    PrimeFactorsDocument.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Mocks ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Mocks PROPERTY FOLDER "src/Mocks")
set_property(TARGET Mocks PROPERTY PROJECT_LABEL "src")
