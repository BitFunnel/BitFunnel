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

#include "BitFunnel/IInterface.h"   // Base class.


namespace BitFunnel
{
    class IFactSet;
    class ITermToText;


    //*************************************************************************
    //
    // IConfiguration is an abstract base class or interface for classes that
    // configure Term construction and maintain a mapping from Term hash to
    // the Term's text.
    //
    //*************************************************************************
    class IConfiguration : public IInterface
    {
    public:
        // Returns the maximum ngram size to be indexed.
        virtual size_t GetMaxGramSize() const = 0;

        // Returns true if the configuration is keeping term text in a
        // TermToText mapping.
        virtual bool KeepTermText() const = 0;

        // Returns the TermToText mapping. This diagnostic class provides
        // a mapping from Term hash to the Term's text in the corpus.
        // Note: behavior is undefined if KeepTermText() is false.
        virtual ITermToText & GetTermToText() const = 0;

        // Returns the IFactSet used to configure the TermTable with user
        // defined fact rows.
        virtual IFactSet const & GetFactSet() const = 0;
    };
}
