# BitFunnel/src/Common/Utilities/src

set(CPPFILES
)

set(WINDOWS_CPPFILES
    BlockingQueueBase.cpp
    ConsoleLogger.cpp
    LockGuard.cpp
    Logging.cpp
    LogLevel.cpp
    MurmurHash2.cpp
    Mutex.cpp
    NullLogger.cpp
    Stopwatch.cpp
    TaskDistributor.cpp
    TaskDistributorThread.cpp
    ThreadManager.cpp
    ThreadSafeCounter.cpp
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
)

set(WINDOWS_PRIVATE_HFILES
    BlockingQueue.h
    LockGuard.h
    MurmurHash2.h
    Mutex.h
    Stopwatch.h
    TaskDistributor.h
    TaskDistributorThread.h
    ThreadManager.h
    ThreadSafeCounter.h
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Utilities ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Utilities PROPERTY FOLDER "src/Common/Utilities")
set_property(TARGET Utilities PROPERTY PROJECT_LABEL "src")
