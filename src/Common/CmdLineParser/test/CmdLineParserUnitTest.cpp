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

#include <iostream>
#include <sstream>
#include <vector>

#include <string.h> // For strncpy

#include "CmdLineParserUnitTest.h"
#include "CmdLineParser/CmdLineParser.h"


#ifdef _MSC_VER
// Disable warning about strncpy. We should probably use c++
// strings and get rid of this, but for now this warning is
// disabled because strncpy_s isn't implemented across
// platforms.
#pragma warning(disable:4996)
#endif // MSC_VER

namespace CmdLine
{
    CmdLineParserUnitTest::CmdLineParserUnitTest(bool createBaseline)
        : AnswerResponseUnitTestBase("CmdLine", "CmdLineParserUnitTest", std::cout, createBaseline, false)
    {
    }


    CmdLineParserUnitTest::~CmdLineParserUnitTest()
    {
    }


    void CmdLineParserUnitTest::RunOneCase(const std::string& input, const std::string& expected)
    {
        int argc;
        char** argv;
        ParseCommandLine(input, argc, argv);

        std::stringstream actual;

        if (argc > 0)
        {
            if (!strcmp(argv[0], "required"))
            {
                TestRequiredParameters(actual, argc, argv);
            }
            else if (!strcmp(argv[0], "optional"))
            {
                TestOptionalParameters(actual, argc, argv);
            }
            else if (!strcmp(argv[0], "constraint"))
            {
                TestConstraints(actual, argc, argv);
            }
            else
            {
                actual << "Error: unknown test case " << argv[0] << "." << std::endl;
            }
        }
        else
        {
            actual << "Error: missing test case name." << std::endl;
        }

        ReportResponse(input, expected, actual.str());

        for (int i = 0; i < argc; ++i)
        {
            free(argv[i]);
        }
        delete [] argv;
    }


    void CmdLineParserUnitTest::TestRequiredParameters(std::ostream& out, int argc, char const* const* argv)
    {
        CmdLineParser parser("RequiredParameterTest", "Test for required parameters.");

        RequiredParameter<int> param1("p1", "First parameter");
        RequiredParameter<double> param2("p2", "Second parameter");
        RequiredParameter<const char*> param3("p3", "Third parameter");
        RequiredParameter<bool> param4("p4", "Fourth parameter");

        parser.AddParameter(param1);
        parser.AddParameter(param2);
        parser.AddParameter(param3);
        parser.AddParameter(param4);

        if (!parser.TryParse(out, argc, argv))
        {
            out << "Parse failure." << std::endl;
        }
        else
        {
            out << "p1 = " << param1 << std::endl;
            out << "p2 = " << param2 << std::endl;
            out << "p3 = " << param3 << std::endl;
            out << "p4 = " << param4 << std::endl;
        }
    }


    void CmdLineParserUnitTest::TestOptionalParameters(std::ostream& out, int argc, char const* const* argv)
    {
        CmdLineParser parser("OptionalParameterTest", "Test for optional parameters.");

        OptionalParameter<int> param1("int", "First parameter", 0);
        OptionalParameter<double> param2("double", "Second parameter", 0);
        OptionalParameter<const char*> param3("string", "Third parameter", "<default>");
        OptionalParameter<bool> param4("bool", "Fourth parameter", false);

        parser.AddParameter(param1);
        parser.AddParameter(param2);
        parser.AddParameter(param3);
        parser.AddParameter(param4);

        OptionalParameterList multi("multi", "Parameter list that requires three values.");
        RequiredParameter<int> m1("first", "First sub-parameter");
        RequiredParameter<double> m2("second", "Second sub-parameter");
        RequiredParameter<bool> m3("third", "Third sub-parameter");
        multi.AddParameter(m1);
        multi.AddParameter(m2);
        multi.AddParameter(m3);
        parser.AddParameter(multi);

        if (!parser.TryParse(out, argc, argv))
        {
            out << "Parse failure." << std::endl;
        }
        else
        {
            out << "int = " << param1 << std::endl;
            out << "double = " << param2 << std::endl;
            out << "string = " << param3 << std::endl;
            out << "bool = " << param4 << std::endl;
            out << "multi = " << multi << std::endl;

            if (multi)
            {
                out << "first = " << m1 << std::endl;
                out << "second = " << m2 << std::endl;
                out << "third = " << m3 << std::endl;
            }
        }
    }


    void CmdLineParserUnitTest::TestConstraints(std::ostream& out, int argc, char const* const* argv)
    {
        CmdLineParser parser("ConstraintTest", "Test for parameter validation and constraints.");

        OptionalParameter<int> lt("lt", "lt parameter", LessThan(10));
        OptionalParameter<int> gt("gt", "gt parameter", GreaterThan(10));
        OptionalParameter<int> lte("lte", "lte parameter", LessThanOrEqual(10));
        OptionalParameter<int> gte("gte", "gte parameter", GreaterThanOrEqual(10));
        OptionalParameter<int> eq("eq", "eq parameter", Equal(10));
        OptionalParameter<int> neq("neq", "neq parameter", NotEqual(10));

        parser.AddParameter(lt);
        parser.AddParameter(gt);
        parser.AddParameter(lte);
        parser.AddParameter(gte);
        parser.AddParameter(eq);
        parser.AddParameter(neq);

        OptionalParameterList a("a", "Parameter a");
        OptionalParameterList b("b", "Parameter b");
        OptionalParameterList c("c", "Parameter c");
        OptionalParameterList d("d", "Parameter d");
        OptionalParameterList e("e", "Parameter e");

        parser.AddParameter(a);
        parser.AddParameter(b);
        parser.AddParameter(c);
        parser.AddParameter(d);
        parser.AddParameter(e);

        parser.AddConstraint(b.Requires(a));
        parser.AddConstraint(MutuallyExclusive(a, c));
        parser.AddConstraint(MutuallyRequired(d, e));

        if (!parser.TryParse(out, argc, argv))
        {
            out << "Parse failure." << std::endl;
        }
        else
        {
            out << "All parameters valid." << std::endl;
        }
    }


    void CmdLineParserUnitTest::ParseCommandLine(std::string cmd, int& argc, char**& argv)
    {
        std::vector<char*> args;

        const char* tail = cmd.c_str();

        while (*tail != 0)
        {
            // Skip over white space.
            while (*tail != 0 && *tail == ' ')
            {
                ++tail;
            }

            if (*tail != 0)
            {
                const char* head = tail;

                // Skip over non-white space.
                while (*tail != 0 && *tail != ' ')
                {
                    tail++;
                }

                // Copy the argument and store in argv array.
                char *arg = new char[tail - head + 1];
                strncpy(arg, head, tail - head);
                arg[tail - head] = 0;
                args.push_back(arg);
            }
        }

        argc = static_cast<int>(args.size());
        argv = new char*[argc];
        for (int i = 0; i < argc; ++i)
        {
            argv[i] = args[i];
        }
    }


    bool CmdLineParserUnitTest::RunAllCases(bool createBaseline)
    {
        CmdLineParserUnitTest test(createBaseline);

        test.InitializeCases();
        return test.RunTest();
    }
}
