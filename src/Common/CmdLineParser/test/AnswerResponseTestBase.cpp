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

#include <ostream>
#include <string.h> // For strdup

#include "AnswerResponseTestBase.h"
#include "gtest/gtest.h"

#ifdef _MSC_VER
// Use cross platform strdup for simplicity.
#pragma warning(disable:4996)
#endif

namespace CmdLine
{
    AnswerResponseTestBase::AnswerResponseTestBase(const char* namespaceName, const char* className, std::ostream& out, bool createBaseline, bool ignoreWhiteSpace)
        : m_out(out), m_createBaseline(createBaseline), m_ignoreWhiteSpace(ignoreWhiteSpace)
    {
        m_namespace = strdup(namespaceName);
        m_className = strdup(className);
    }


    AnswerResponseTestBase::~AnswerResponseTestBase()
    {
        free(m_namespace);
        free(m_className);
    }


    void AnswerResponseTestBase::AddCase(const char* input, const char* expected)
    {
        m_input.push_back(std::string(input));
        m_expected.push_back(std::string(expected));
    }


    bool AnswerResponseTestBase::RunTest()
    {
        m_passedCount = 0;

        if (m_createBaseline)
        {
            EmitPrologue();
        }

        for (unsigned i = 0; i < m_input.size(); ++i)
        {
            RunOneCase(m_input[i], m_expected[i]);
        }

        if (m_createBaseline)
        {
            EmitEpilogue();
        }
        else
        {
            m_out << m_className << ": Passed " << m_passedCount << " out of " << m_input.size() << " cases.\n";
        }

        return (m_passedCount == m_input.size());
    }


    void AnswerResponseTestBase::ReportResponse(const std::string& input, const std::string& expected, const std::string& actualResponse)
    {
        if (m_createBaseline)
        {
            EmitOneCase(input, actualResponse);
            ++m_passedCount;
        }
        else
        {
            bool result = ValidateResponse(expected, actualResponse);
            if (result)
            {
                ++m_passedCount;
            }
            else
            {
                m_out << "CASE FAILED:\n";
                m_out << input;
                m_out << "\nEXPECTED:\n";
                m_out << expected;
                m_out << "\nFOUND:\n";
                m_out << actualResponse;
                m_out << "\n--------------\n";
            }
            ASSERT_TRUE(result);
        }
    }


    void AnswerResponseTestBase::EmitPrologue()
    {
        m_out << "#include \"stdafx.h\"\n\n";
        m_out << "#include \"" << m_className << ".h\"\n\n";
        m_out << "namespace " << m_namespace << "\n{\n";
        m_out << "    void " << m_className << "::InitializeCases()\n";
        m_out << "    {\n";
    }


    void AnswerResponseTestBase::EmitOneCase(const std::string& input, const std::string& expected)
    {
        const char addText[] = "        AddCase(";
        const unsigned indent = sizeof(addText) - 1;

        m_out << addText;
        EmitStringLiteral(indent, input);
        m_out << ", ";

        m_out << "\n";
        Indent(indent);

        EmitStringLiteral(indent, expected);
        m_out << ");\n";
    }


    void AnswerResponseTestBase::Indent(unsigned count)
    {
        if (count > 0)
        {
            m_out << std::setw(count) << ' ';
        }
    }


    unsigned AnswerResponseTestBase::EmitStringLiteral(unsigned indent, const std::string& s)
    {
        unsigned lineCounter = 0;
        m_out << '"';

        unsigned i = 0;
        while (i < s.length())
        {
            if (s[i] == '\n')
            {
                m_out << "\\n";
                ++i;

                if (i < s.length())
                {
                    m_out << "\"\n";
                    Indent(indent);
                    m_out << "\"";
                    ++lineCounter;
                }
            }
            else if (s[i] == '"')
            {
                m_out << "\\\"";
                ++i;
            }
            else if (s[i] == '\\')
            {
                m_out << "\\\\";
                ++i;
            }
            else if (s[i] >= 32)
            {
                m_out << s[i];
                ++i;
            }
            else
            {
                throw std::runtime_error("C++ escaping only supports \\n, \", and \\.");
            }
        }

        m_out << '"';

        return lineCounter;
    }

    void AnswerResponseTestBase::EmitEpilogue()
    {
        m_out << "    }\n";
        m_out << "}\n";
    }

    // Returns true when response and m_response differ only in white space.
    bool AnswerResponseTestBase::ValidateResponse(const std::string& input, const std::string& expected) const
    {
        const char* p1 = input.c_str();
        const char* p2 = expected.c_str();

        if (m_ignoreWhiteSpace)
        {
            while (*p1 != 0 && *p2 != 0)
            {
                SkipWhite(p1);
                SkipWhite(p2);
                if (*p1 != *p2)
                {
                    break;
                }
                if (*p1 != 0)
                {
                    ++p1;
                }
                if (*p2 != 0)
                {
                    ++p2;
                }
            }

            return *p1 == 0 && *p2 == 0;
        }
        else
        {
            return !strcmp(p1, p2);
        }
    }

    void AnswerResponseTestBase::SkipWhite(const char*& s)
    {
        while (*s == '\n' || *s == '\t' || *s == '\r' || *s == ' ')
        {
            s++;
        }
    }
}
