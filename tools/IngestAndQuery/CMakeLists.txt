# BitFunnel/tools/IngestAndQuery

set(CPPFILES
    main.cpp
    TaskFactory.cpp
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

# Horrible hack to allow this to instantiate anything.
# TODO: figure out how this should really work.
include_directories(${CMAKE_SOURCE_DIR}/src/Common/Utilities/src)
include_directories(${CMAKE_SOURCE_DIR}/src/Index/src)
include_directories(${CMAKE_SOURCE_DIR}/test/Shared)


add_executable(IngestAndQuery ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries(IngestAndQuery CmdLineParser TestShared Index Configuration CsvTsv Utilities)
set_property(TARGET IngestAndQuery PROPERTY FOLDER "tools")
set_property(TARGET IngestAndQuery PROPERTY PROJECT_LABEL "IngestAndQuery")
