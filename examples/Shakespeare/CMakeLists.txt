# BitFunnel/examples/Shakespeare

set(CPPFILES
    main.cpp
    Sonnets.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    Sonnets.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()


add_executable(Shakespeare ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries(Shakespeare)
set_property(TARGET Shakespeare PROPERTY FOLDER "examples")
set_property(TARGET Shakespeare PROPERTY PROJECT_LABEL "Shakespeare")
