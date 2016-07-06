# BitFunnel/src/Common/Utilities/src

set(CPPFILES
    AlignedBuffer.cpp
    BlockAllocator.cpp
    ConsoleLogger.cpp
    Exceptions.cpp
    FileHeader.cpp
    Logging.cpp
    LogLevel.cpp
    MurmurHash2.cpp
    NullLogger.cpp
    Rounding.cpp
    SimpleHashPolicy.cpp
    SimpleHashSet.cpp
    SimpleHashSetBase.cpp
    StandardInputStream.cpp
    StreamUtilities.cpp
    TaskDistributor.cpp
    TaskDistributorThread.cpp
    ThreadManager.cpp
    Token.cpp
    TokenManager.cpp
    TokenTracker.cpp
    Version.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    AlignedBuffer.h
    BlockAllocator.h
    BlockingQueue.h
    MurmurHash2.h
    Rounding.h
    SimpleHashPolicy.h
    SimpleHashSet.h
    SimpleHashSetBase.h
    SimpleHashTable.h
    TaskDistributor.h
    TaskDistributorThread.h
    TokenManager.h
    TokenTracker.h
    ThreadManager.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Utilities ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Utilities PROPERTY FOLDER "src/Common/Utilities")
set_property(TARGET Utilities PROPERTY PROJECT_LABEL "src")
