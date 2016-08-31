# BitFunnel/test/Shared

set(CPPFILES
    IndexUtils.cpp
    EmptyTermTable.cpp
    FakeSliceOwner.cpp
    MockFileManager.cpp
    MockTermTable.cpp
    SameExceptForWhitespace.cpp
)

set(PRIVATE_HFILES
)

set(PUBLIC_HFILES
    IndexUtils.h
    EmptyTermTable.h
    FakeSliceOwner.h
    MockFileManager.h
    MockTermTable.h
    SameExceptForWhitespace.h
)

COMBINE_FILE_LISTS()

# include_directories(${PROJECT_SOURCE_DIR}/src/BitFunnel)
include_directories(${PROJECT_SOURCE_DIR}/src/Common/Utilities/src)
include_directories(${PROJECT_SOURCE_DIR}/src/Index/src)
include_directories(${PROJECT_SOURCE_DIR}/test/Shared)

add_library(TestShared ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET TestShared PROPERTY FOLDER "test")
