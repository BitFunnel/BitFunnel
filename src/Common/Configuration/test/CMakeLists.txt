# BitFunnel/src/Common/Configuration/test

set(CPPFILES
    ShardDefinitionTest.cpp
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
include_directories(${CMAKE_SOURCE_DIR}/src/Common/Configuration/src)


add_executable(ConfigurationTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET ConfigurationTest PROPERTY FOLDER "src/Common/Configuration")
set_property(TARGET ConfigurationTest PROPERTY PROJECT_LABEL "Test")
target_link_libraries (ConfigurationTest Configuration Utilities gtest gtest_main)

add_test(NAME ConfigurationTest COMMAND ConfigurationTest)
