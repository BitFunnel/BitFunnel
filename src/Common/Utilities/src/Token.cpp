#include "Token.h"
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
