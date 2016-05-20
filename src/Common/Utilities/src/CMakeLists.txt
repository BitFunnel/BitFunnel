# BitFunnel/src/Common/Utilities/src

set(CPPFILES
    MurmurHash2.cpp
    Mutex.cpp
    ThreadSafeCounter.cpp
    TaskDistributor.cpp
)

set(WINDOWS_CPPFILES
    BlockingQueueBase.cpp
    ConsoleLogger.cpp
    LockGuard.cpp
    Logging.cpp
    LogLevel.cpp
    NullLogger.cpp
    Stopwatch.cpp
    TaskDistributorThread.cpp
    ThreadManager.cpp
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    MurmurHash2.h
    Mutex.h
    TaskDistributor.h
    ThreadsafeCounter.h
)

set(WINDOWS_PRIVATE_HFILES
    BlockingQueue.h
    LockGuard.h
    Stopwatch.h
    TaskDistributorThread.h
    ThreadManager.h
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Utilities ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Utilities PROPERTY FOLDER "src/Common/Utilities")
set_property(TARGET Utilities PROPERTY PROJECT_LABEL "src")
