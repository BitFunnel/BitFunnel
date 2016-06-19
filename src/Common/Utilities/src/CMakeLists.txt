# BitFunnel/src/Common/Utilities/src

set(CPPFILES
    ConsoleLogger.cpp
    Logging.cpp
    LogLevel.cpp
    MurmurHash2.cpp
    NullLogger.cpp
    SimpleHashSetBase.cpp
    TaskDistributor.cpp
    TaskDistributorThread.cpp
    ThreadManager.cpp
    Token.cpp
    TokenManager.cpp
    TokenTracker.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    MurmurHash2.h
    SimpleHashSetBase.h
    TaskDistributor.h
    TaskDistributorThread.h
    Token.h
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
