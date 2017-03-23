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

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "BitFunnel/NonCopyable.h"


namespace CmdLine
{
    class AnswerResponseTestBase : public BitFunnel::NonCopyable
    {
    public:
        AnswerResponseTestBase(const char* namespaceName,
                                   const char* className,
                                   std::ostream& out,
                                   bool createBaseline,
                                   bool ignoreWhiteSpace);

        virtual ~AnswerResponseTestBase();

        bool RunTest();

    protected:
        void AddCase(const char* input, const char* expected);
        virtual void InitializeCases() = 0;
        virtual void RunOneCase(const std::string& input,
                                const std::string& expected) = 0;
        void ReportResponse(const std::string& input,
                            const std::string& expected,
                            const std::string& actualResponse);

    private:
        void EmitPrologue();
        void EmitOneCase(const std::string& input, const std::string& expected);
        unsigned EmitStringLiteral(unsigned indent, const std::string& s);
        void Indent(unsigned nSpaces);
        void EmitEpilogue();

        bool ValidateResponse(const std::string& input,
                              const std::string& expected) const;
        static void SkipWhite(const char*& s);


        char* m_namespace;
        char* m_className;

        std::ostream& m_out;
        bool m_createBaseline;
        bool m_ignoreWhiteSpace;

        std::vector<std::string> m_input;
        std::vector<std::string> m_expected;

        unsigned m_passedCount;
    };
}
