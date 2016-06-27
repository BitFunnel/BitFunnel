#include <sstream>

#include "gtest/gtest.h"

#include "BitFunnel/Utilities/FileHeader.h"
#include "BitFunnel/Utilities/Version.h"

namespace BitFunnel
{
    namespace FileHeaderUnitTest
    {
        TEST(Roundtrip, Trivial)
        {
            Version version(1, 1, 0);
            FileHeader fileHeader(version, "Hello World");
            std::stringstream stream;
            fileHeader.Write(stream);
            FileHeader fileHeader2(stream);
            EXPECT_EQ(fileHeader2.GetVersion().VersionMajor(),
                      fileHeader.GetVersion().VersionMajor());
            EXPECT_EQ(fileHeader2.GetVersion().VersionMiddle(),
                      fileHeader.GetVersion().VersionMiddle());
            EXPECT_EQ(fileHeader2.GetVersion().VersionMinor(),
                      fileHeader.GetVersion().VersionMinor());
            EXPECT_EQ(fileHeader2.TimeStamp(),
                      fileHeader.TimeStamp());
            EXPECT_EQ(fileHeader2.UserData(),
                      fileHeader.UserData());

            std::stringstream stream2;
            fileHeader2.Write(stream2);

            EXPECT_EQ(stream.str(), stream2.str());

        }
    }
}
