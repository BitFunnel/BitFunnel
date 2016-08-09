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

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/ITermTable.h"

// TODO: Remove this temporary include. 
#include "BitFunnel/Row.h"



namespace BitFunnel
{
    // TODO: Remove this temporary StreamId.
    const Term::StreamId c_metaWord = 0;

    // Helper function to create a system internal term. System internal terms
    // are special terms whose Raw hash values by convention are sequential
    // numbers starting at 0. The number of such terms is specified in
    // c_systemRowCount.
    Term CreateSystemInternalTerm(Term::Hash rawHash)
    {
        if (rawHash >= c_systemRowCount)
        {
            RecoverableError error("Hash out of internal row range.");
            throw error;
        }

        const Term term(rawHash,
                        c_metaWord,
                        static_cast<Term::IdfX10>(0));
        return term;
    }


    //*************************************************************************
    //
    // Some random ITermTable static methods.
    // TODO: figure out where these should go.
    //
    //*************************************************************************
    Term ITermTable::GetSoftDeletedTerm()
    {
        static Term softDeletedTerm(CreateSystemInternalTerm(0));
        return softDeletedTerm;
    }


    Term ITermTable::GetMatchAllTerm()
    {
        static Term matchAllTerm(CreateSystemInternalTerm(1));
        return matchAllTerm;
    }


    Term ITermTable::GetMatchNoneTerm()
    {
        static Term matchNoneTerm(CreateSystemInternalTerm(2));
        return matchNoneTerm;
    }
}
