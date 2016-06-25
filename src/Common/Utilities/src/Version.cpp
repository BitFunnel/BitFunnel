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
        *this = Version::Version(standardIn);
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
            << m_versionMajor
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
