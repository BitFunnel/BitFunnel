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

#include <sstream>

#include "AddShardTermWriter.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Configuration/IShardDefinition.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // AddShardTermWriter
    //
    //*************************************************************************

    AddShardTermWriter::AddShardTermWriter(IFileManager * fileManager, IIngestor & ingestor)
      : m_fileManager(fileManager),
        m_ingestor(ingestor)
    {
    }

    void AddShardTermWriter::SetChunk(size_t index)
    {
        m_output = m_fileManager->Chunk(index).OpenForWrite();
    }

    void AddShardTermWriter::WriteDoc(BitFunnel::IDocument & document, char const * start, size_t size)
    {
        // We need min/max count info about the shard the document's posting count belongs in
        // (Note: We add one to the posting count to account for the added shard term)
        const BitFunnel::IShardDefinition & sharddef = m_ingestor.GetShardDef();
        auto shardid = sharddef.GetShard(document.GetPostingCount()+1);
        auto mincnt = sharddef.GetMinPostingCount(shardid);
        auto maxcnt = sharddef.GetMaxPostingCount(shardid);
        std::ostringstream shardterm;

        // Write out all of document except last document-ending byte
        m_output->write(start, size-1);

        // Write out shard term into stream 00 (default for body)
        *m_output << "00" << '\0' << "SHARD_" << mincnt << "_" << maxcnt << '\0' << '\0' << '\0';
    }


    void AddShardTermWriter::Complete()
    {
        *m_output << '\0';
    }
}
