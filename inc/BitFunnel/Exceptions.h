#pragma once

#include <stdexcept>    // Inherits from std::runtime_error.
#include <string>       // std::string parameter.

// TODO: Add logging method.

namespace BitFunnel
{
    class RecoverableError : public std::runtime_error
    {
    public:
        RecoverableError(char const * msg);
        RecoverableError(std::string const & msg);
        RecoverableError();
    };


    class FatalError : public std::runtime_error
    {
    public:
        FatalError(char const * msg);
        FatalError(std::string const & msg);
        FatalError();
    };


    class NotImplemented : public FatalError
    {
    public:
        NotImplemented(char const * msg);
        NotImplemented(std::string const & msg);
        NotImplemented();
    };
}
