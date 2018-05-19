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

#pragma once

#include <iosfwd>                           // std::istream parameter.
#include <string>                           // std::string embedded, template parameter.
#include <unordered_map>                    // std::unordered_map embedded.

#include "BitFunnel/Index/ITermToText.h"    // Base class.
#include "BitFunnel/Term.h"                 // Term::Hash parameter.


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermToText
    //
    // A mapping from Term::Hash to an std::string with the text representation
    // of the term. Used primarily for debugging and understanding index data
    // structures.
    //
    //*************************************************************************
    class TermToText : public ITermToText
    {
    public:
        // Constructs an empty map. New (Term::Hash, std::string) pairs can
        // be added via AddTerm().
        TermToText();

        // Constructs a map from data previously persisted via Write().
        TermToText(std::istream & input);

        //
        // ITermToText methods.
        //

        // Adds a (Term::Hash, std::string) mapping. Note that only the first
        // mapping for a particular Term::Hash will be recorded. Subsequent
        // additions for the same Term::Hash will be ignored.
        virtual void AddTerm(Term::Hash hash, std::string const & text) override;

        // Returns the text for a particular Term::Hash, if that hash is in the
        // map. Otherwise returns an empty string.
        virtual std::string const & Lookup(Term::Hash hash) const override;

        // Persists the map to a stream. Data format is .csv with the following
        // columns:
        //   hash: Hexidecimal representation of the term hash
        //   text: Unquoted term text. May contain spaces if term's ngram size
        //         is greater than 1.
        virtual void Write(std::ostream& output) const override;

    private:
        // Empty string returned by Lookup() when hash is not in the map.
        // Implemented as a member because Lookup() returns a const reference.
        const std::string m_emptyString;

        // Term::Hash ==> std::string map.
        std::unordered_map<Term::Hash, std::string> m_termToText;
    };
}
