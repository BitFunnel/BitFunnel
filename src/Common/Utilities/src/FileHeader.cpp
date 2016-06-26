#include <cstring>
#include <ctime>
#include <time.h>

#include "BitFunnel/Utilities/FileHeader.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "BitFunnel/Utilities/Version.h"
#include "LoggerInterfaces/Logging.h"

#ifdef _MSC_VER
#define sscanf sscanf_s
#define timegm _mkgmtime
#endif


namespace BitFunnel
{
    FileHeader::FileHeader(const Version& version, const std::string& userData)
        : m_version(new Version(version.VersionMajor(), version.VersionMiddle(), version.VersionMinor())),
          m_time(new FileHeaderTime()),
          m_userData(new FileHeaderData(userData))
    {
    }

    FileHeader::FileHeader(std::istream& in)
    {
        Read(in);
    }

    FileHeader::~FileHeader()
    {
        delete m_userData;
        delete m_time;
        delete m_version;
    }

    void FileHeader::Read(std::istream& in)
    {
        m_version = new Version(in);
        m_time = new FileHeaderTime(in);
        char ch = StreamUtilities::ReadField<char>(in);
        std::string errorMessage = "Read expected '\n', found ";
        errorMessage += ch;
        LogAssertB(ch == '\n',
                   errorMessage.c_str());
        m_userData = new FileHeaderData(in);
    }

    void FileHeader::Write(std::ostream& out) const
    {
        m_version->Write(out);
        m_time->Write(out);
        char ch = '\n';
        StreamUtilities::WriteField<char>(out, ch);
        m_userData->Write(out);
    }

    const Version& FileHeader::GetVersion() const
    {
        return *m_version;
    }

    const time_t& FileHeader::TimeStamp() const
    {
        return m_time->TimeStamp();
    }

    const std::string& FileHeader::UserData() const
    {
        return m_userData->Data();
    }

    FileHeaderTime::FileHeaderTime()
    {
        time(&m_time);
    }

    const time_t& FileHeaderTime::TimeStamp() const
    {
        return m_time;
    }

    FileHeaderTime::FileHeaderTime(std::istream& in)
    {
        const int c_len = (int)strlen("04/20/2011 02:24:45");
        char buffer[64];
        StreamUtilities::ReadBytes(in, buffer, c_len);
        buffer[c_len] = 0;
        struct tm tms;
        LogAssertB(sscanf(buffer,
                          "%02d/%2d/%04d %02d:%02d:%02d",
                          &tms.tm_mon,
                          &tms.tm_mday,
                          &tms.tm_year,
                          &tms.tm_hour,
                          &tms.tm_min,
                          &tms.tm_sec) == 6,
                   "sscanf failed to read header time.");
        tms.tm_year -= 1900;
        m_time = timegm(&tms);
    }

    void FileHeaderTime::Write(std::ostream& out) const
    {
        // write in character format as: 04/20/2011 02:24:45
        char buffer[64];
        struct tm tms;
#ifdef _MSC_VER
        LogAssertB(gmtime_s(&tms, &m_time) != 0,
                   "Bad time passed to gmtime_s.");
        sprintf_s(buffer,
                  64,
                  "%02d/%2d/%04d %02d:%02d:%02d",
                  tms.tm_mon,
                  tms.tm_mday,
                  tms.tm_year + 1900,
                  tms.tm_hour,
                  tms.tm_min,
                  tms.tm_sec);
#else
        sprintf(buffer,
                "%02d/%2d/%04d %02d:%02d:%02d",
                tms.tm_mon,
                tms.tm_mday,
                tms.tm_year + 1900,
                tms.tm_hour,
                tms.tm_min,
                tms.tm_sec);
        LogAssertB(gmtime_r(&m_time, &tms) != NULL,
                   "Bad time passed to gmtime_r.");
#endif
        StreamUtilities::WriteBytes(out, buffer, strlen(buffer));
    }

    FileHeaderData::FileHeaderData(const std::string& data)
        : m_data(data)
    {
        LogAssertB(data.length() < c_maxUserDataLen,
                   "FileHeaderData length >= c_maxUserDataLen");
    }

    const std::string& FileHeaderData::Data() const
    {
        return m_data;
    }

    FileHeaderData::FileHeaderData(std::istream& in)
    {
        // Read character by character until the null character 0.
        char* buffer = new char[c_maxUserDataLen];
        int len = 0;
        while (len < c_maxUserDataLen)
        {
            buffer[len] = StreamUtilities::ReadField<char>(in);
            if (buffer[len] == 0)
            {
                break;
            }
            len++;
        }
        LogAssertB(len < c_maxUserDataLen,
                   "FileHeaderData len >= maxUserDataLen");
        m_data = buffer;
        delete [] buffer;
    }

    void FileHeaderData::Write(std::ostream& out) const
    {   
        StreamUtilities::WriteBytes(out, m_data.c_str(), m_data.length());
        char ch = 0;
        StreamUtilities::WriteField<char>(out, ch);
    }
}
