# BitFunnel/src/Plan/test

set(CPPFILES
    CompileNodeTest.cpp
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

# Unit tests are allowed to access private headers of the library they test.
include_directories(${CMAKE_SOURCE_DIR}/src/Plan/src)

# TODO: fix this hack.
include_directories(${CMAKE_SOURCE_DIR}/test/Shared)

add_executable(PlanTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET PlanTest PROPERTY FOLDER "src/Plan")
set_property(TARGET PlanTest PROPERTY PROJECT_LABEL "Test")

# NOTE: The ordering Utilities-Plan is important for XCode. If you reverse
# Utilities and Plan, we will get linker errors.
# TODO: do we really need Configuration?
# TODO: do we need CsvTsv?
target_link_libraries (PlanTest TestShared Utilities Plan Configuration  Utilities  gtest gtest_main)

add_test(NAME PlanTest COMMAND PlanTest)
