#include "BitFunnel/Exceptions.h"


namespace BitFunnel
{
    RecoverableError::RecoverableError(char const * msg)
        : std::runtime_error(msg)
    {
    }


    RecoverableError::RecoverableError(std::string const & msg)
        : std::runtime_error(msg)
    {
    }


    RecoverableError::RecoverableError()
        : std::runtime_error("")
    {
    }


    FatalError::FatalError(char const * msg)
        : std::runtime_error(msg)
    {
    }


    FatalError::FatalError(std::string const & msg)
        : std::runtime_error(msg)
    {
    }


    FatalError::FatalError()
        : std::runtime_error("")
    {
    }


    NotImplemented::NotImplemented(char const * msg)
        : FatalError(msg)
    {
    }


    NotImplemented::NotImplemented(std::string const & msg)
        : FatalError(msg)
    {
    }


    NotImplemented::NotImplemented()
        : FatalError()
    {
    }



}