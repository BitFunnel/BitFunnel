#pragma once

#include "BitFunnel/AbstractRow.h"
#include "LoggerInterfaces/Logging.h"
#include "NodeUtilities.h"


namespace BitFunnel
{
    template <>
    inline AbstractRow ParseField<AbstractRow>(IObjectParser& parser)
    {
        return AbstractRow(parser, false);
    }


    template <class T>
    static T const & ParseBinaryTree(IObjectParser& parser, const char* childrenFieldName)
    {
        parser.OpenObject();
        parser.OpenObjectField(childrenFieldName);

        parser.OpenList();

        LogAssertB(parser.OpenListItem(),
                   "binary nodes must have exactly two children; only found one?");
        T const & node = dynamic_cast<T const &>(ParseList<CompileNode, T>(parser));

        parser.CloseList();
        parser.CloseObject();

        return node;
    }
}
