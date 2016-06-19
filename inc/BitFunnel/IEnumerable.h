#pragma once

#include "BitFunnel/IEnumerator.h"

namespace BitFunnel
{
    template <typename T>
    class IEnumerable
    {
    public:
        virtual ~IEnumerable() {};

        virtual IEnumerator<T>* GetEnumerator() const = 0;
    };
}
