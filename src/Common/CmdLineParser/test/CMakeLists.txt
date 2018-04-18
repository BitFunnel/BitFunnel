# BitFunnel/src/Common/CmdLineParser/test

set(CPPFILES
    AnswerResponseTestBase.cpp
    AnswerResponseTestBase.h
    CmdLineParserCases.cpp
    CmdLineParserTest.cpp
    CmdLineParserTest.h
    Main.cpp
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

add_executable(CmdLineParserTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET CmdLineParserTest PROPERTY FOLDER "src/Common/CmdLineParser")
set_property(TARGET CmdLineParserTest PROPERTY PROJECT_LABEL "Test")
target_link_libraries (CmdLineParserTest CmdLineParser gtest gtest_main)

add_test(NAME CmdLineParserTest COMMAND CmdLineParserTest)
