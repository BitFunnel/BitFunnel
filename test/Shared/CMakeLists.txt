# BitFunnel/test/Shared

set(CPPFILES
  IndexUtils.cpp
  EmptyTermTable.cpp
  MockFileManager.cpp
  MockTermTable.cpp
)

set(PRIVATE_HFILES
)

set(PUBLIC_HFILES
  IndexUtils.h
  EmptyTermTable.h
  MockFileManager.h
  MockTermTable.h
)

COMBINE_FILE_LISTS()

# include_directories(${PROJECT_SOURCE_DIR}/src/BitFunnel)
include_directories(${PROJECT_SOURCE_DIR}/src/Common/Utilities/src)
include_directories(${PROJECT_SOURCE_DIR}/src/Index/src)

add_library(TestShared ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET TestShared PROPERTY FOLDER "test")
