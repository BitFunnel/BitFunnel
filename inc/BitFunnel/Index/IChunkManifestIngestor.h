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

#include <stddef.h>                 // size_t return value.
#include "BitFunnel/IInterface.h"   // Base class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // IChunkManifestIngestor
    //
    // Abstract base class or interface for classes representing a set of
    // chunk data. Each chunk represents a set of documents.
    //
    //*************************************************************************
    class IChunkManifestIngestor : public IInterface
    {
    public:
        // Returns the number of chunks in this manifest.
        virtual size_t GetChunkCount() const = 0;

        // Ingests the specified chunk.
        // NOTE that parameters controlling ingestion are supplied to the
        // constructor of the object that implements IChunkManifestIngestor.
        virtual void IngestChunk(size_t index) const = 0;
    };
}
