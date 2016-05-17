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


namespace CmdLine
{
    class FormattingUtilities
    {
    public:
        // Prints level spaces to the out stream.
        static void Indent(std::ostream& out, unsigned level);

        // Formats the contents of two stringstreams into two rectangles.
        // The contents of left is placed in a rectangle that is leftColumn
        // characters wide. The contents of right is placed in a rectangle
        // to the right that extends out to targetWidth characters from the
        // left margin.
        static void FormatTextBlock(std::stringstream& left,
                                    std::stringstream& right,
                                    std::ostream& out,
                                    unsigned leftColumn,
                                    unsigned targetWidth);

        static void Format(std::ostream& out, int value);
        static void Format(std::ostream& out, double value);
        static void Format(std::ostream& out, bool value);
        static void Format(std::ostream& out, const char* value);
    };
}
