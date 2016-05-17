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

#include "CmdLineParserUnitTest.h"

namespace CmdLine
{
    void CmdLineParserUnitTest::InitializeCases()
    {
        AddCase("required", 
                "Expected <p1>.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("required -help", 
                "RequiredParameterTest\n"
                "Test for required parameters.\n"
                "\n"
                "Usage: \n"
                "CmdLineParserUnitTest.exe <p1>\n"
                "                          <p2>\n"
                "                          <p3>\n"
                "                          <p4>\n"
                "                          [-help]\n"
                "\n"
                "<p1>\n"
                "    First parameter (integer)\n"
                "\n"
                "<p2>\n"
                "    Second parameter (floating point)\n"
                "\n"
                "<p3>\n"
                "    Third parameter (string)\n"
                "\n"
                "<p4>\n"
                "    Fourth parameter (boolean)\n"
                "\n"
                "[-help]\n"
                "    Display help for this program. (boolean, defaults to false)\n"
                "\n"
                "\n"
                "\n"
                "Parse failure.\n");
        AddCase("required 1 2.0 three true -help", 
                "RequiredParameterTest\n"
                "Test for required parameters.\n"
                "\n"
                "Usage: \n"
                "CmdLineParserUnitTest.exe <p1>\n"
                "                          <p2>\n"
                "                          <p3>\n"
                "                          <p4>\n"
                "                          [-help]\n"
                "\n"
                "<p1>\n"
                "    First parameter (integer)\n"
                "\n"
                "<p2>\n"
                "    Second parameter (floating point)\n"
                "\n"
                "<p3>\n"
                "    Third parameter (string)\n"
                "\n"
                "<p4>\n"
                "    Fourth parameter (boolean)\n"
                "\n"
                "[-help]\n"
                "    Display help for this program. (boolean, defaults to false)\n"
                "\n"
                "\n"
                "\n"
                "Parse failure.\n");
        AddCase("required 1 2.0 three true", 
                "p1 = 1\n"
                "p2 = 2\n"
                "p3 = \"three\"\n"
                "p4 = true\n");
        AddCase("required -1 -2.0 -three true", 
                "p1 = -1\n"
                "p2 = -2\n"
                "p3 = \"-three\"\n"
                "p4 = true\n");
        AddCase("required 2.0", 
                "Expected integer parameter <p1>.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("required 2.1", 
                "Expected integer parameter <p1>.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("required foobar", 
                "Expected integer parameter <p1>.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional", 
                "int = 0\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -help", 
                "OptionalParameterTest\n"
                "Test for optional parameters.\n"
                "\n"
                "Usage: \n"
                "CmdLineParserUnitTest.exe [-help]\n"
                "                          [-int <integer>]\n"
                "                          [-double <floating point>]\n"
                "                          [-string <string>]\n"
                "                          [-bool <boolean>]\n"
                "                          [-multi <first> <second> <third>]\n"
                "\n"
                "[-help]\n"
                "    Display help for this program. (boolean, defaults to false)\n"
                "\n"
                "\n"
                "[-int <integer>]\n"
                "    First parameter (integer, defaults to 0)\n"
                "\n"
                "[-double <floating point>]\n"
                "    Second parameter (floating point, defaults to 0)\n"
                "\n"
                "[-string <string>]\n"
                "    Third parameter (string, defaults to \"<default>\")\n"
                "\n"
                "[-bool <boolean>]\n"
                "    Fourth parameter (boolean, defaults to false)\n"
                "\n"
                "[-multi <first> <second> <third>]\n"
                "    Parameter list that requires three values. (boolean, defaults to false)\n"
                "\n"
                "  <first>   First sub-parameter (integer)\n"
                "\n"
                "  <second>  Second sub-parameter (floating point)\n"
                "\n"
                "  <third>   Third sub-parameter (boolean)\n"
                "\n"
                "\n"
                "\n"
                "Parse failure.\n");
        AddCase("optional -int 1", 
                "int = 1\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -int 0", 
                "int = 0\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -int -9", 
                "int = -9\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -int +5", 
                "int = 5\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -int 007", 
                "int = 7\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -int 123456", 
                "int = 123456\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -int", 
                "Expected integer value for optional parameter -int.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -int 2.1", 
                "Expected integer value for optional parameter -int.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -int foobar", 
                "Expected integer value for optional parameter -int.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -double 0.1", 
                "int = 0\n"
                "double = 0.1\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -double .1", 
                "int = 0\n"
                "double = 0.1\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -double -10", 
                "int = 0\n"
                "double = -10\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -double -1.2345e5", 
                "int = 0\n"
                "double = -123450\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -double", 
                "Expected floating point value for optional parameter -double.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -double e", 
                "Expected floating point value for optional parameter -double.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -string -string", 
                "int = 0\n"
                "double = 0\n"
                "string = \"-string\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -string -bool", 
                "int = 0\n"
                "double = 0\n"
                "string = \"-bool\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -string text", 
                "int = 0\n"
                "double = 0\n"
                "string = \"text\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -string 123", 
                "int = 0\n"
                "double = 0\n"
                "string = \"123\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -string ", 
                "Expected string value for optional parameter -string.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool true", 
                "int = 0\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = true\n"
                "multi = false\n");
        AddCase("optional -bool false", 
                "int = 0\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = false\n");
        AddCase("optional -bool foobar", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool 1234", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool TRUE", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool FALSE", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool yes", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool no", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool 1", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool 0", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -bool ", 
                "Expected boolean value for optional parameter -bool.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -int 1 -int 2", 
                "Found second instance of optional argument [-int <integer>].\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -multi 1 2.0 true", 
                "int = 0\n"
                "double = 0\n"
                "string = \"<default>\"\n"
                "bool = false\n"
                "multi = true\n"
                "first = 1\n"
                "second = 2\n"
                "third = true\n");
        AddCase("optional -multi 1 2.0", 
                "Expected boolean parameter <third>.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("optional -multi true 1 2.0", 
                "Expected integer parameter <first>.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -help", 
                "ConstraintTest\n"
                "Test for parameter validation and constraints.\n"
                "\n"
                "Usage: \n"
                "CmdLineParserUnitTest.exe [-help]\n"
                "                          [-lt <integer>]\n"
                "                          [-gt <integer>]\n"
                "                          [-lte <integer>]\n"
                "                          [-gte <integer>]\n"
                "                          [-eq <integer>]\n"
                "                          [-neq <integer>]\n"
                "                          [-a]\n"
                "                          [-b]\n"
                "                          [-c]\n"
                "                          [-d]\n"
                "                          [-e]\n"
                "\n"
                "[-help]\n"
                "    Display help for this program. (boolean, defaults to false)\n"
                "\n"
                "\n"
                "[-lt <integer>]\n"
                "    lt parameter (integer, must be less than 10)\n"
                "\n"
                "[-gt <integer>]\n"
                "    gt parameter (integer, must be greater than 10)\n"
                "\n"
                "[-lte <integer>]\n"
                "    lte parameter (integer, must be less than or equal to 10)\n"
                "\n"
                "[-gte <integer>]\n"
                "    gte parameter (integer, must be greater than or equal to 10)\n"
                "\n"
                "[-eq <integer>]\n"
                "    eq parameter (integer, must be equal 10)\n"
                "\n"
                "[-neq <integer>]\n"
                "    neq parameter (integer, cannot equal 10)\n"
                "\n"
                "[-a]\n"
                "    Parameter a (boolean, defaults to false)\n"
                "\n"
                "\n"
                "[-b]\n"
                "    Parameter b (boolean, defaults to false)\n"
                "\n"
                "\n"
                "[-c]\n"
                "    Parameter c (boolean, defaults to false)\n"
                "\n"
                "\n"
                "[-d]\n"
                "    Parameter d (boolean, defaults to false)\n"
                "\n"
                "\n"
                "[-e]\n"
                "    Parameter e (boolean, defaults to false)\n"
                "\n"
                "\n"
                "\n"
                "Constraints\n"
                "  Optional parameter -b requires -a.\n"
                "  Optional parameter -a is incompatible with -c.\n"
                "  Optional parameters -d and -e must be used together..\n"
                "Parse failure.\n");
        AddCase("constraint -lt 5", 
                "All parameters valid.\n");
        AddCase("constraint -lt 10", 
                "Validation error: lt must be less than 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -lt 20", 
                "Validation error: lt must be less than 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -gt 5", 
                "Validation error: gt must be greater than 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -gt 10", 
                "Validation error: gt must be greater than 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -gt 20", 
                "All parameters valid.\n");
        AddCase("constraint -lte 5", 
                "All parameters valid.\n");
        AddCase("constraint -lte 10", 
                "All parameters valid.\n");
        AddCase("constraint -lte 20", 
                "Validation error: lte must be less than or equal to 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -gte 5", 
                "Validation error: gte must be greater than or equal to 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -gte 10", 
                "All parameters valid.\n");
        AddCase("constraint -gte 20", 
                "All parameters valid.\n");
        AddCase("constraint -eq 5", 
                "Validation error: eq must be equal 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -eq 10", 
                "All parameters valid.\n");
        AddCase("constraint -eq 20", 
                "Validation error: eq must be equal 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -neq 5", 
                "All parameters valid.\n");
        AddCase("constraint -neq 10", 
                "Validation error: neq cannot equal 10.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -neq 20", 
                "All parameters valid.\n");
        AddCase("constraint -a", 
                "All parameters valid.\n");
        AddCase("constraint -b", 
                "Constraint not satisfied: Optional parameter -b requires -a.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -b -a", 
                "All parameters valid.\n");
        AddCase("constraint -c", 
                "All parameters valid.\n");
        AddCase("constraint -a -c", 
                "Constraint not satisfied: Optional parameter -a is incompatible with -c.\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -e", 
                "Constraint not satisfied: Optional parameters -d and -e must be used together..\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -d", 
                "Constraint not satisfied: Optional parameters -d and -e must be used together..\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
        AddCase("constraint -d -e", 
                "All parameters valid.\n");
        AddCase("constraint -a -a", 
                "Found second instance of optional argument [-a].\n"
                "Use -help for more information.\n"
                "Parse failure.\n");
    }
}
