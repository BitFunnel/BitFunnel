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

#include <stdint.h>
#include <vector>

#include "BitFunnel/Chunks/IChunkProcessor.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/NonCopyable.h"


namespace BitFunnel
{

    //*************************************************************************
    //
    // AddShardTermWriter
    //
    // Outputs a buffer of documents encoded in the BitFunnel chunk format
    // to an output stream. Importantly, it also injects into each document
    // a searchable term that identifies the shard the document belongs to.
    //
    //*************************************************************************
    class AddShardTermWriter : public NonCopyable, public IChunkWriter
    {
    public:
        AddShardTermWriter(IFileManager * fileManager, IIngestor & ingestor);

        // Sets the index of the chunk to be processed.
        // This creates the stream from the fileManager used to output the current chunk
        void SetChunk(size_t index) override;

        // Writes the document's bytes (in range m_start, m_end) to the current chunk's stream.
        void WriteDoc(BitFunnel::IDocument & document, char const * start, size_t size) override;

        // Writes a single '\0' to the specified stream, completing the outputted chunk.
        void Complete() override;

    private:
        IFileManager * m_fileManager;
        IIngestor & m_ingestor;
        std::unique_ptr<std::ostream> m_output;
    };
}
