#include "DiagnosticStream.h"
#include "BitFunnel/Utilities/Factories.h"


namespace BitFunnel
{
    std::unique_ptr<IDiagnosticStream> Factories::CreateDiagnosticStream(std::ostream& stream)
    {
        return std::unique_ptr<IDiagnosticStream>(new DiagnosticStream(stream));
    }


    DiagnosticStream::DiagnosticStream(std::ostream& stream)
        : m_stream(stream)
    {
    }


    void DiagnosticStream::Enable(char const * diagnostic)
    {
        // TODO: REVIEW: check for duplicates?
        m_enabled.push_back(diagnostic);
    }


    void DiagnosticStream::Disable(char const * diagnostic)
    {
        // TODO: REVIEW: check for mismatch?
        for (unsigned i = 0 ; i < m_enabled.size(); ++i)
        {
            if (m_enabled[i].compare(diagnostic) == 0)
            {
                m_enabled.erase(m_enabled.begin() + i);

                // TODO: REVIEW: Assuming there is only one to remove.
                break;
            }
        }
    }


    // Returns true if text starts with prefix.
    bool StartsWith(char const * text, std::string const & prefix)
    {
        for (unsigned i = 0 ; i < prefix.size(); ++i)
        {
            if (*text != prefix[i])
            {
                return false;
            }
            ++text;
        }
        return true;
    }


    bool DiagnosticStream::IsEnabled(char const * diagnostic) const
    {
        for (unsigned i = 0; i < m_enabled.size(); ++i)
        {
            if (StartsWith(diagnostic, m_enabled[i]))
            {
                return true;
            }
        }
        return false;
    }


    std::ostream& DiagnosticStream::GetStream()
    {
        return m_stream;
    }
}
