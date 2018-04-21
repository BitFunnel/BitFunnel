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

#include "BitFunnel/Chunks/Factories.h"
#include "BitFunnel/Configuration/IShardDefinition.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/IDocument.h"

#include "ChunkWriters.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************
    std::unique_ptr<IChunkWriterFactory>
        Factories::CreateCopyingChunkWriterFactory(IFileManager& fileManager)
    {
        return std::make_unique<CopyingChunkWriterFactory>(fileManager);
    }


    std::unique_ptr<IChunkWriterFactory>
        Factories::CreateAnnotatingChunkWriterFactory(
            IFileManager& fileManager,
            IShardDefinition const & shardDefinition)
    {
        return std::make_unique<AnnotatingChunkWriterFactory>(
            fileManager,
            shardDefinition);
    }


    //*************************************************************************
    //
    // CopyingChunkWriter
    //
    //*************************************************************************
    CopyingChunkWriter::CopyingChunkWriter(IFileManager& fileManager, size_t index)
        : m_output(fileManager.Chunk(index).OpenForWrite())
    {
    }


    CopyingChunkWriter::~CopyingChunkWriter()
    {
        // Zero-terminate the stream, indicating that there are no more documents.
        // The stream itself will be closed when its std::unique_ptr is destructed.
        *m_output << '\0';
    }


    void CopyingChunkWriter::Write(IDocument const &, char const * start, size_t length)
    {
        m_output->write(start, length);
    }


    //*************************************************************************
    //
    // CopyingChunkWriterFactory
    //
    //*************************************************************************
    CopyingChunkWriterFactory::CopyingChunkWriterFactory(IFileManager& fileManager)
        : m_fileManager(fileManager)
    {
    }


    std::unique_ptr<IChunkWriter>
        CopyingChunkWriterFactory::CreateChunkWriter(size_t index)
    {
        return std::make_unique<CopyingChunkWriter>(m_fileManager, index);
    }


    //*************************************************************************
    //
    // AnnotatingChunkWriter
    //
    //*************************************************************************
    AnnotatingChunkWriter::AnnotatingChunkWriter(
        IFileManager& fileManager,
        size_t index,
        IShardDefinition const & shardDefinition)
        : CopyingChunkWriter(fileManager, index),
          m_shardDefinition(shardDefinition)
    {
    }

    void AnnotatingChunkWriter::Write(IDocument const & document,
                                      char const * start,
                                      size_t length)
    {
        // We need min/max count info about the shard the document's posting count belongs in
        // (Note: We add one to the posting count to account for the added shard term)
        auto shardId = m_shardDefinition.GetShard(document.GetPostingCount() + 1);
        auto minCount = m_shardDefinition.GetMinPostingCount(shardId);
        auto maxCount = m_shardDefinition.GetMaxPostingCount(shardId);

        // Write out all of document except last document-ending byte
        m_output->write(start, length - 1);

        // Write out shard term into stream 00 (default for body)
        *m_output << "00" << '\0'
            << "SHARD_" << minCount << "_" << maxCount << '\0' << '\0' << '\0';
    }


    //*************************************************************************
    //
    // AnnotatingChunkWriterFactory
    //
    //*************************************************************************
    AnnotatingChunkWriterFactory::AnnotatingChunkWriterFactory(
        IFileManager& fileManager,
        IShardDefinition const & shardDefinition)
        : CopyingChunkWriterFactory(fileManager),
          m_shardDefinition(shardDefinition)
    {
    }


    std::unique_ptr<IChunkWriter>
        AnnotatingChunkWriterFactory::CreateChunkWriter(size_t index)
    {
        return std::make_unique<AnnotatingChunkWriter>(m_fileManager,
                                                       index,
                                                       m_shardDefinition);
    }
}
