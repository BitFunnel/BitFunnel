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

#include "BitFunnel/AbstractRow.h"
#include "LoggerInterfaces/Logging.h"
#include "NodeUtilities.h"


namespace BitFunnel
{
    template <>
    inline Classification ParseField<Classification>(IObjectParser& parser)
    {
        std::string classificationName;
        parser.ParseToken(classificationName);
        return StringToClassification(classificationName);
    }


    template <>
    inline Term ParseField<Term>(IObjectParser& parser)
    {
        return Term(parser, false);
    }


    template <>
    inline AbstractRow ParseField<AbstractRow>(IObjectParser& parser)
    {
        return AbstractRow(parser, false);
    }


    // Allow trailing primitive items of string type be optional.
    // E.g. unigram parser supports optional stream suffix:
    //  Unigram("term", full)               - suffix is nullptr;
    //  Unigram("term", full, "stream")     - suffix is "stream";
    inline char const * ParseOptionalStringPrimitiveItem(IObjectParser& parser)
    {
        if (parser.OpenPrimitiveItem())
        {
            return ParseField<char const *>(parser);
        }
        else
        {
            return nullptr;
        }
    }


    // Allow object field with optional value.
    // E.g. phrase parser supports optional stream suffix:
    //  Suffix: nullable()          - suffix is nullptr;
    //  Suffix: nullable("stream")  - suffix is "stream";
    inline char const * ParseNullableObjectStringField(IObjectParser& parser, char const *name)
    {
        parser.OpenObjectField(name);
        parser.OpenPrimitive("nullable");
        char const * result = ParseOptionalStringPrimitiveItem(parser);
        parser.ClosePrimitive();
        return result;
    }
}
