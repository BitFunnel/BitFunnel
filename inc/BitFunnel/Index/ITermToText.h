#pragma once

#include <string>                   // std::string return value.

#include "BitFunnel/IInterface.h"   // Base class.
#include "BitFunnel/Term.h"         // Term::Hash parameter.


namespace BitFunnel
{
    //*************************************************************************
    //
    // ITermToText
    //
    // Abstract base class or interface for classes implementing a mapping from
    // Term::Hash to an std::string with the text representation of the term.
    // Used primarily for debugging and understanding index data structures.
    //
    //*************************************************************************
    class ITermToText : public IInterface
    {
    public:
        // Returns the text for a particular Term::Hash, if that hash is in the
        // map. Otherwise returns an empty string.
        virtual std::string const & Lookup(Term::Hash hash) const = 0;
    };
}
