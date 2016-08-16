#pragma once

#include "BitFunnel/Term.h"       // For Term::Hash.

namespace BitFunnel
{
    //*************************************************************************
    //
    // ICommonPhrases is an abstract base class or interface for classes that
    // maintain a set of commonly occurring phrases, indexed by classified
    // hash.
    //
    //*************************************************************************
    class ICommonPhrases
    {
    public:
        virtual ~ICommonPhrases() {};

        // Returns true if CommonPhrases contains the term with a specified
        // classified hash.
        virtual bool IsCommon(Term::Hash classifiedHash) const = 0;
    };
}
