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

#include "BitFunnel/Utilities/TextObjectFormatter.h"
#include "PlainTextCodeGenerator.h"


namespace BitFunnel
{
    PlainTextCodeGenerator::PlainTextCodeGenerator(std::ostream& output)
        : m_output(output),
          m_label(0)
    {
    }


    void PlainTextCodeGenerator::EmitZeroArg(char const* name)
    {
        Indent();
        m_output << name << "()" << std::endl;
    }

    void PlainTextCodeGenerator::EmitSignedArg(char const* name, int shift)
    {
        Indent();
        m_output << name << "(" << shift << ")" << std::endl;
    }


    void PlainTextCodeGenerator::EmitUnsignedArg(char const* name,
                                                 unsigned shift)
    {
        Indent();
        m_output << name << "(" << shift << ")" << std::endl;
    }


    void PlainTextCodeGenerator::EmitSizeTArg(char const* name,
                                              size_t shift)
    {
        Indent();
        m_output << name << "(" << shift << ")" << std::endl;
    }


    void PlainTextCodeGenerator::EmitRowArg(char const* name,
                                            size_t id,
                                            bool inverted,
                                            size_t rankDelta)
    {
        Indent();
        m_output << name << "(" << id;
        if (inverted)
        {
            m_output << ", true";
        }
        else
        {
            m_output << ", false";
        }
        m_output << ", " << rankDelta;
        m_output << ")" << std::endl;
    }


    void PlainTextCodeGenerator::LoadRow(size_t id, bool inverted, size_t rankDelta)
    {
        EmitRowArg("LoadRow", id, inverted, rankDelta);
    }


    void PlainTextCodeGenerator::AndRow(size_t id, bool inverted, size_t rankDelta)
    {
        EmitRowArg("AndRow", id, inverted, rankDelta);
    }


    void PlainTextCodeGenerator::LeftShiftOffset(size_t shift)
    {
        EmitSizeTArg("LeftShiftOffset", shift);
    }


    void PlainTextCodeGenerator::RightShiftOffset(size_t shift)
    {
        EmitSizeTArg("RightShiftOffset", shift);
    }


    void PlainTextCodeGenerator::IncrementOffset()
    {
        EmitZeroArg("IncrementOffset");
    }


    void PlainTextCodeGenerator::Push()
    {
        EmitZeroArg("Push");
    }


    void PlainTextCodeGenerator::Pop()
    {
        EmitZeroArg("Pop");
    }


    void PlainTextCodeGenerator::AddStack()
    {
        EmitZeroArg("AddStack");
    }


    void PlainTextCodeGenerator::AndStack()
    {
        EmitZeroArg("AndStack");
    }


    void PlainTextCodeGenerator::Constant(int value)
    {
        EmitSignedArg("Constant", value);
    }


    void PlainTextCodeGenerator::MaxStack()
    {
        EmitZeroArg("MaxStack");
    }


    void PlainTextCodeGenerator::Not()
    {
        EmitZeroArg("Not");
    }


    void PlainTextCodeGenerator::OrStack()
    {
        EmitZeroArg("OrStack");
    }


    void PlainTextCodeGenerator::UpdateFlags()
    {
        EmitZeroArg("UpdateFlags");
    }


    void PlainTextCodeGenerator::Report()
    {
        EmitZeroArg("Report");
    }


    PlainTextCodeGenerator::Label PlainTextCodeGenerator::AllocateLabel()
    {
        return m_label++;
    }


    void PlainTextCodeGenerator::PlaceLabel(Label label)
    {
        m_output << "L" << label << ":" << std::endl;
    }


    void PlainTextCodeGenerator::Call(Label label)
    {
        EmitSizeTArg("Call", label);
    }


    void PlainTextCodeGenerator::Jmp(Label label)
    {
        EmitSizeTArg("Jmp", label);
    }


    void PlainTextCodeGenerator::Jnz(Label label)
    {
        EmitSizeTArg("Jnz", label);
    }


    void PlainTextCodeGenerator::Jz(Label label)
    {
        EmitSizeTArg("Jz", label);
    }


    void PlainTextCodeGenerator::Return()
    {
        EmitZeroArg("Return");
    }

    void PlainTextCodeGenerator::Indent()
    {
        m_output << "    ";
    }
}
