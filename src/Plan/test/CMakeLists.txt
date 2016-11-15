# BitFunnel/src/Plan/test

set(CPPFILES
    # AbstractRowEnumeratorTest.cpp
    AbstractRowTest.cpp
    ByteCodeInterpreterTest.cpp
    ByteCodeVerifier.cpp
    CodeVerifierBase.cpp
    CompileNodeTest.cpp
    MatchTreeRewriterTest.cpp
    NativeCodeVerifier.cpp
    NativeCodeTest.cpp
    PlainTextCodeGenerator.cpp
    RankDownCompilerTest.cpp
    RegisterAllocatorTest.cpp
    RowPlanTest.cpp
    QueryParserTest.cpp
    TermMatchNodeTest.cpp
    TermPlanConverterTest.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    ByteCodeVerifier.h
    CodeVerifierBase.h
    ICodeVerifier.h
    NativeCodeVerifier.h
    PlainTextCodeGenerator.h
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
target_link_libraries (PlanTest TestShared Mocks Plan Index Chunks Configuration Utilities CsvTsv NativeJIT CodeGen gtest gtest_main)

add_test(NAME PlanTest COMMAND PlanTest)
