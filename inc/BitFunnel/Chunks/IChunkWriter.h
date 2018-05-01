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
#include "BitFunnel/NonCopyable.h"  // Base class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // IChunkWriter
    //
    // Interface passed to IChunkProcessor to aid in copying a subset of
    // documents from a chunk file. IChunkWriter provides methods for writing
    // documents in the format of the reader that generates IChunkProcessor
    // callbacks.
    //
    //*************************************************************************
    class IChunkWriter : public IInterface, NonCopyable
    {
    public:
        // Writes the document.
        // The start and length parameters refer to a buffer containing the
        // BitFunnel corpus/chunk file encoding of the document to be written.
        virtual void Write(IDocument const & document,
                           char const * start,
                           size_t length) = 0;
    };


    //*************************************************************************
    //
    // IChunkWriterFactory
    //
    // Interface that constructs IChunkWriters. Typically one IChunkWriter is
    // constructed for each corpus/chunk file processed.
    //
    //*************************************************************************
    class IChunkWriterFactory : public IInterface, NonCopyable
    {
    public:
        virtual std::unique_ptr<IChunkWriter>
            CreateChunkWriter(size_t index) = 0;
    };
}
