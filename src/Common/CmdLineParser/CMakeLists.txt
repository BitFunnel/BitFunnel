# BitFunnel/src/Common/CmdLineParser

add_subdirectory(src)
add_subdirectory(UnitTest)

set(PUBLIC_HFILES
  ${CMAKE_SOURCE_DIR}/src/Common/CmdLineParser/inc/CmdLineParser/CmdLineParser.h
  ${CMAKE_SOURCE_DIR}/src/Common/CmdLineParser/inc/CmdLineParser/Constraint.h
  ${CMAKE_SOURCE_DIR}/src/Common/CmdLineParser/inc/CmdLineParser/FormattingUtilities.h
  ${CMAKE_SOURCE_DIR}/src/Common/CmdLineParser/inc/CmdLineParser/Parameter.h
  ${CMAKE_SOURCE_DIR}/src/Common/CmdLineParser/inc/CmdLineParser/Validator.h
)

source_group("CmdLineParser" FILES ${PUBLIC_HFILES})

#
# Create a custom_target for the public header file directory.
#
add_custom_target(CmdLineParserInc SOURCES ${PUBLIC_HFILES})
set_property(TARGET CmdLineParserInc PROPERTY FOLDER "src/Common/CmdLineParser")
set_property(TARGET CmdLineParserInc PROPERTY PROJECT_LABEL "inc")

