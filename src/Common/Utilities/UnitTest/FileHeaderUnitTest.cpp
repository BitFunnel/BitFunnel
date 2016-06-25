#include "stdafx.h"

#include <sstream>

#include "BitFunnel/FileHeader.h"
#include "BitFunnel/Version.h"
#include "SuiteCpp/UnitTest.h"

namespace BitFunnel
{
    namespace FileHeaderUnitTest
    {
        TestCase(Roundtrip)
        {
            Version version(1, 1, 0);
            FileHeader fileHeader(version, "Hello World");
            std::stringstream stream;
            fileHeader.Write(stream);

            FileHeader fileHeader2(stream);

            TestAssert(fileHeader2.GetVersion().VersionMajor() == fileHeader.GetVersion().VersionMajor());
            TestAssert(fileHeader2.GetVersion().VersionMiddle() == fileHeader.GetVersion().VersionMiddle());
            TestAssert(fileHeader2.GetVersion().VersionMinor() == fileHeader.GetVersion().VersionMinor());
            TestAssert(fileHeader2.TimeStamp() == fileHeader.TimeStamp());
            TestAssert(strcmp(fileHeader2.UserData().c_str(), fileHeader.UserData().c_str()) == 0);
        }
    }
}
