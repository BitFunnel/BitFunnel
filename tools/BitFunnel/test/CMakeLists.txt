# BitFunnel/tools/BitFunnel/test

set(CPPFILES
    BitFunnelToolTest.cpp
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
include_directories(${CMAKE_SOURCE_DIR}/tools/BitFunnel/src)

add_executable(BitFunnelToolTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET BitFunnelToolTest PROPERTY FOLDER "tools/BitFunnel")
set_property(TARGET BitFunnelToolTest PROPERTY PROJECT_LABEL "Test")

# NOTE: The ordering Utilities-Index is important for XCode. If you reverse
# Utilities and Index, we will get linker errors.
target_link_libraries (BitFunnelToolTest BitFunnelTool Data CmdLineParser CsvTsv Plan Utilities Index Chunks Configuration CsvTsv Utilities NativeJIT CodeGen gtest gtest_main)

add_test(NAME BitFunnelToolTest COMMAND BitFunnelToolTest)
