# BitFunnel/src

# BitFunnel libraries are allowed to access private headers of
# the Utilities library.
include_directories(
  Common/Utilities/src
)

#
# Libraries
#
add_subdirectory(Common/CmdLineParser)
add_subdirectory(Common/Configuration)
add_subdirectory(Common/CsvTsv)
add_subdirectory(Common/Utilities)
add_subdirectory(Index)

