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
