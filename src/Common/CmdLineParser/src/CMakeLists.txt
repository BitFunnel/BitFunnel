# BitFunnel/src/Common/CmdLineParser/src

set(CPPFILES
)

set(WINDOWS_CPPFILES
    CmdLineParser.cpp
    Constraint.cpp
    FormattingUtilities.cpp
    Parameter.cpp
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

add_library(CmdLineParser ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET CmdLineParser PROPERTY FOLDER "src/Common/CmdLineParser")
set_property(TARGET CmdLineParser PROPERTY PROJECT_LABEL "src")
