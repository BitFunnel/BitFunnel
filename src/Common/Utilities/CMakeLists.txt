# BitFunnel/src/Common/Utilities

if (BITFUNNEL_PLATFORM_WINDOWS)
    add_subdirectory(src)
    add_subdirectory(UnitTest)

    source_group("Utilities" FILES ${PUBLIC_HFILES})

endif (BITFUNNEL_PLATFORM_WINDOWS)
