#include <sstream>

#include "gtest/gtest.h"

#include "BitFunnel/Utilities/Version.h"


namespace BitFunnel
{
    namespace VersionUnitTest
    {
        TEST(Manual, Trivial)
        {
            Version version(1, 1, 0);

            std::stringstream output;
            version.Write(output);

            Version version2(output);

            EXPECT_EQ(version.VersionMajor(), version2.VersionMajor());
            EXPECT_EQ(version.VersionMiddle(), version2.VersionMiddle());
            EXPECT_EQ(version.VersionMinor(), version2.VersionMinor());

            Version version3(1, 1, 1);
            EXPECT_TRUE(version.IsCompatibleWith(version3));

            Version version4(1, 2, 0);
            EXPECT_TRUE(!version.IsCompatibleWith(version4));

            Version version5(2, 1, 0);
            EXPECT_TRUE(!version.IsCompatibleWith(version5));

            Version version6 = version.IncrementMajor();
            EXPECT_EQ(version6.VersionMajor(), 2);
            EXPECT_EQ(version6.VersionMiddle(), 0);
            EXPECT_EQ(version6.VersionMinor(), 0);

            Version version7 = version.IncrementMiddle();
            EXPECT_EQ(version7.VersionMajor(), 1);
            EXPECT_EQ(version7.VersionMiddle(), 2);
            EXPECT_EQ(version7.VersionMinor(), 0);

            Version version8 = version.IncrementMinor();
            EXPECT_EQ(version8.VersionMajor(), 1);
            EXPECT_EQ(version8.VersionMiddle(), 1);
            EXPECT_EQ(version8.VersionMinor(), 1);
        }
    }
}
