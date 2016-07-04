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


#include "BitFunnel/Token.h"
#include "LoggerInterfaces/Logging.h"

namespace BitFunnel
{
    Token::Token(ITokenListener& issuer, SerialNumber serialNumber)
        : m_issuer(&issuer),
          m_serialNumber(serialNumber)
    {
    }


    Token::Token(Token && other)
        : m_issuer(other.m_issuer),
          m_serialNumber(other.m_serialNumber)
    {
        other.m_issuer = nullptr;
    }


    Token::~Token()
    {
        if (m_issuer == nullptr)
        {
            // This is a zombie token copy, it was moved away to a new location.
            return;
        }

        // It's most important to make sure every token that ends its lifetime
        // reports to its issuer. We also need to try to avoid unhandled
        // exceptions in the destructor.
        try
        {
            m_issuer->OnTokenComplete(m_serialNumber);
        }
        catch (...)
        {
            LogB(Logging::Error,
                 "Token",
                 "Exception caught while destroying a Token",
                 "");
        }
    }


    SerialNumber Token::GetSerialNumber() const
    {
        return m_serialNumber;
    }
}
