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
