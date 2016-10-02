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

#include <memory>                       // std::unique_ptr return value.

#include "BitFunnel/BitFunnelTypes.h"   // DocId parameter.
#include "BitFunnel/Term.h"             // Term::StreamId parameter.

namespace BitFunnel
{
    class IConfiguration;
    class IDocument;
    class IFileSystem;

    //*************************************************************************
    //
    // CreatePrimeFactorsDocument
    //
    //*************************************************************************
    std::unique_ptr<IDocument>
        CreatePrimeFactorsDocument(IConfiguration const & config, DocId id);


    //*************************************************************************
    //
    // CreatePrimeFactorsTermTable
    //
    //*************************************************************************
    std::unique_ptr<ITermTable>
        CreatePrimeFactorsTermTable(DocId maxDocId,
                                    Term::StreamId /*streamId*/);


    //*************************************************************************
    //
    // CreatePrimeFactorsIndex
    //
    //*************************************************************************
    std::unique_ptr<ISimpleIndex>
        CreatePrimeFactorsIndex(IFileSystem & fileSystem,
                                DocId maxDocId,
                                Term::StreamId /*streamId*/,
                                IConfiguration const & /*config*/);
}
