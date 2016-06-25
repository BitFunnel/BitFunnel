#include "stdafx.h"

#include <sstream>

#include "BitFunnel/Version.h"
#include "SuiteCpp/UnitTest.h"


namespace BitFunnel
{
    namespace VersionUnitTest
    {
        TestCase(Comprehensive)
        {
            Version version(1, 1, 0);

            std::stringstream output;
            version.Write(output);

            Version version2(output);

            TestAssert(version.VersionMajor() == version2.VersionMajor());
            TestAssert(version.VersionMiddle() == version2.VersionMiddle());
            TestAssert(version.VersionMinor() == version2.VersionMinor());

            Version version3(1, 1, 1);
            TestAssert(version.IsCompatibleWith(version3));

            Version version4(1, 2, 0);
            TestAssert(!version.IsCompatibleWith(version4));

            Version version5(2, 1, 0);
            TestAssert(!version.IsCompatibleWith(version5));

            Version version6 = version.IncrementMajor();
            TestAssert(version6.VersionMajor() == 2);
            TestAssert(version6.VersionMiddle() == 0);
            TestAssert(version6.VersionMinor() == 0);

            Version version7 = version.IncrementMiddle();
            TestAssert(version7.VersionMajor() == 1);
            TestAssert(version7.VersionMiddle() == 2);
            TestAssert(version7.VersionMinor() == 0);

            Version version8 = version.IncrementMinor();
            TestAssert(version8.VersionMajor() == 1);
            TestAssert(version8.VersionMiddle() == 1);
            TestAssert(version8.VersionMinor() == 1);
        }
    }
}
