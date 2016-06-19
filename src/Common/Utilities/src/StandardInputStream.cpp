#include "stdafx.h"

#include <istream>

#include "BitFunnel/StandardInputStream.h"


namespace BitFunnel
{
    StandardInputStream::StandardInputStream(std::istream& stream)
        : m_stream(stream)
    {
    }


    size_t StandardInputStream::Read(char* destination, size_t byteCount)
    {
        m_stream.read(destination, byteCount);

        return m_stream.gcount();
    }
}
