# BitFunnel/tools/StatisticsBuilder

set(CPPFILES
    main.cpp
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

add_executable(StatisticBuilder ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries(StatisticBuilder CmdLineParser Index Utilities)
set_property(TARGET StatisticBuilder PROPERTY FOLDER "tools")
set_property(TARGET StatisticBuilder PROPERTY PROJECT_LABEL "StatisticsBuilder")
