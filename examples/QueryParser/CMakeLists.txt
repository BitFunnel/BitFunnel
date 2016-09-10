# BitFunnel/examples/QueryParser

set(CPPFILES
    main.cpp
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


add_executable(QueryParser ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries(QueryParser Utilities Plan)
set_property(TARGET QueryParser PROPERTY FOLDER "examples")
set_property(TARGET QueryParser PROPERTY PROJECT_LABEL "QueryParser")
