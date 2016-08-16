#pragma once

#include "BitFunnel/IObjectFormatter.h"


namespace BitFunnel
{
    namespace ObjectFormatter
    {
        template <class LIST>
        void FormatListField(IObjectFormatter& formatter,
                             const char* name,
                             const LIST& list)
        {
            formatter.OpenObjectField(name);
            formatter.OpenList();

            const LIST::Node* node = list.GetHead();
            FormatListHelper<LIST::Node>(formatter, node);

            formatter.CloseList();
        }


        template <class NODE>
        void FormatListHelper(IObjectFormatter& formatter,
                              NODE const * node)
        {
            if (node != nullptr)
            {
                FormatListHelper(formatter, node->GetNext());

                formatter.OpenListItem();
                node->GetValue().Format(formatter);
            }
        }
    }
}
