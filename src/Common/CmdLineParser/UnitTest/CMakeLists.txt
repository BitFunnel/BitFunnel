# BitFunnel/src/Common/CmdLineParser/UnitTest

set(CPPFILES
    AnswerResponseUnitTestBase.cpp
    AnswerResponseUnitTestBase.h
    CMakeLists.txt
    CmdLineParserCases.cpp
    CmdLineParserUnitTest.cpp
    CmdLineParserUnitTest.h
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

add_executable(CmdLineParserUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET CmdLineParserUnitTest PROPERTY FOLDER "src/Common/CmdLineParser")
set_property(TARGET CmdLineParserUnitTest PROPERTY PROJECT_LABEL "UnitTest")
target_link_libraries (CmdLineParserUnitTest CmdLineParser gtest gtest_main)

add_test(NAME CmdLineParserUnitTest COMMAND CmdLineParserUnitTest)
