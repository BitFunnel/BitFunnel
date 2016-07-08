// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <istream>
#include <ostream>
#include <string>
#include <sstream>

#include "BitFunnel/Utilities/StandardInputStream.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "BitFunnel/Utilities/Version.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    Version::Version(int major, int middle, int minor)
        : m_versionMajor(major),
          m_versionMiddle(middle),
          m_versionMinor(minor)
    {
        LogAssertB(major >= 0 && middle >= 0 && minor >= 0,
                   "Negative version number");
    }

    Version::Version(std::istream& in)
    {
        StandardInputStream standardIn(in);

        // To avoid compiler warning 4239, create a temporary version
        // object using StandardInputStream and copy to this object.
        // Since the Version object is very small, no perf impact is expected.
        *this = Version(standardIn);
    }


    Version::Version(IInputStream& in)
    {
        // We expect format like "1.23.4 " as used by FileHeader.
        int value = -1;  // -1 to indicate no value yet.
        bool readMajor = false;
        bool readMiddle = false;
        bool readMinor = false;
        char ch = 0;
        while (ch != ' ')
        {
            // TODO: Why not read directly from stream instead of calling ReadField?
            ch = StreamUtilities::ReadField<char>(in);
            if (ch == ' ')
            {
                LogAssertB(readMajor && readMiddle && !readMinor,
                           "Bad state in reading Version.");
                LogAssertB(value >= 0,
                           "Read negative version number.");
                m_versionMinor = value;
                break;
            }
            if (ch == '.')
            {
                LogAssertB(value >= 0,
                           "Read negative version number.");
                LogAssertB(!readMajor || !readMiddle,
                           "Bad state in reading Version.");
                if (!readMajor)
                {
                    m_versionMajor = value;
                    readMajor = true;
                    value = -1;
                }
                else
                {
                    m_versionMiddle = value;
                    readMiddle = true;
                    value = -1;
                }
            }
            else
            {
                LogAssertB(ch >= '0' && ch <= '9',
                           "Unexpected non-dig while reading Version.");
                if (value == -1)
                {
                    value = (ch - '0');
                }
                else
                {
                    value = 10 * value + (ch - '0');
                }
                LogAssertB(value >= 0,
                           "Read negative version number.");
            }
        }
    }

    void Version::Write(std::ostream& out) const
    {
        // Write version in character string format like "1.23.4 ", as
        // expected by FileHeader.
        std::stringstream buf;
        buf << m_versionMajor
            << "."
            << m_versionMiddle
            << "."
            << m_versionMinor
            << " ";

        std::string outputVersion = buf.str();
        LogAssertB(outputVersion.length() < 64,
                   "Unexpectedly long Version::Write");
        StreamUtilities::WriteBytes(out, outputVersion.c_str(), outputVersion.length());
    }

    bool Version::IsCompatibleWith(const Version& other) const
    {
        return (m_versionMajor == other.m_versionMajor && m_versionMiddle == other.m_versionMiddle);
    }

    int Version::VersionMajor() const
    {
        return m_versionMajor;
    }

    int Version::VersionMiddle() const
    {
        return m_versionMiddle;
    }

    int Version::VersionMinor() const
    {
        return m_versionMinor;
    }

    Version Version::IncrementMajor() const
    {
        return Version(m_versionMajor + 1, 0, 0);
    }

    Version Version::IncrementMiddle() const
    {
        return Version(m_versionMajor, m_versionMiddle + 1, 0);
    }

    Version Version::IncrementMinor() const
    {
        return Version(m_versionMajor, m_versionMiddle, m_versionMinor + 1);
    }
}
