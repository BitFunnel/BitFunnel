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

#include "BitFunnel/Index/IIngestor.h"
#include "ChunkIngestor.h"
#include "Document.h"


namespace BitFunnel
{
    ChunkIngestor::ChunkIngestor(
        std::vector<char> const & chunkData,
        IConfiguration const & config,
        IIngestor& ingestor)
      : m_config(config),
        m_ingestor(ingestor),
        m_chunkData(chunkData)
    {
        ChunkReader(m_chunkData, *this);
    }


    void ChunkIngestor::OnFileEnter()
    {
    }


    void ChunkIngestor::OnDocumentEnter(DocId /*id*/)
    {
        m_currentDocument.reset(new Document(m_config));
    }


    void ChunkIngestor::OnStreamEnter(char const * name)
    {
        m_currentDocument->OpenStream(name);
    }


    void ChunkIngestor::OnTerm(char const * term)
    {
        m_currentDocument->AddTerm(term);
    }


    void ChunkIngestor::OnStreamExit()
    {
        m_currentDocument->CloseStream();
    }


    void ChunkIngestor::OnDocumentExit()
    {
        m_ingestor.Add(0, *m_currentDocument);
        m_currentDocument.reset(nullptr);
    }


    void ChunkIngestor::OnFileExit()
    {
    }
}
