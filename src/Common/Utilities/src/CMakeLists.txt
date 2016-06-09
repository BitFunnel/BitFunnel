# BitFunnel/src/Common/Utilities/src

set(CPPFILES
    ConsoleLogger.cpp
    Logging.cpp
    LogLevel.cpp
    MurmurHash2.cpp
    NullLogger.cpp
    TaskDistributor.cpp
    TaskDistributorThread.cpp
    ThreadManager.cpp
    ThreadsafeCounter.cpp
)

set(WINDOWS_CPPFILES
    Stopwatch.cpp
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    MurmurHash2.h
    TaskDistributor.h
    TaskDistributorThread.h
    ThreadManager.h
    ThreadsafeCounter.h
)

set(WINDOWS_PRIVATE_HFILES
    Stopwatch.h
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Utilities ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Utilities PROPERTY FOLDER "src/Common/Utilities")
set_property(TARGET Utilities PROPERTY PROJECT_LABEL "src")
