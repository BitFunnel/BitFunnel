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

#include <sstream>

#include "CmdLineParser/FormattingUtilities.h"


namespace CmdLine
{
    void FormattingUtilities::Indent(std::ostream& out, unsigned level)
    {
        if (level > 0)
        {
            out.width(level);
            out.fill(' ');
            out << ' ';
        }
    }


    void FormattingUtilities::FormatTextBlock(std::stringstream& left,
                                              std::stringstream& right,
                                              std::ostream& out,
                                              unsigned leftWidth,
                                              unsigned rightWidth)
    {
        left.clear();
        left.seekg(0);

        unsigned charCount = 0;
        while (left.peek() != std::char_traits<char>::eof())
        {
            out << static_cast<char>(left.get());
            ++charCount;
        }
        Indent(out, leftWidth - charCount);

        right.clear();
        right.seekg(0);

        unsigned position = leftWidth;

        while (right.peek() != std::char_traits<char>::eof())
        {
            if (right.peek() == '\n')
            {
                right.get();
                out << std::endl;
                Indent(out, leftWidth);
                position = leftWidth;
            }
            else
            {
                char c = static_cast<char>(right.get());
                if ((c == ' ') && (position > (leftWidth + rightWidth)))
                {
                    out << std::endl;
                    Indent(out, leftWidth);
                    position = leftWidth;
                }
                else
                {
                    out << c;
                    ++position;
                }
            }
        }
    }


    void FormattingUtilities::Format(std::ostream& out, int value)
    {
        out << value;
    }


    void FormattingUtilities::Format(std::ostream& out, double value)
    {
        out << value;
    }


    void FormattingUtilities::Format(std::ostream& out, bool value)
    {
        if (value)
        {
            out << "TRUE";
        }
        else
        {
            out << "FALSE";
        }
    }


    // TODO: REVIEW: Should this escape quotation marks within strings?
    void FormattingUtilities::Format(std::ostream& out, const char* value)
    {
        out << '"' << value << '"';
    }
}
