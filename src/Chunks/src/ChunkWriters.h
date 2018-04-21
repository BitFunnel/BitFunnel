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

#include <iosfwd>                           // std::ostream template parameter.
#include <memory>                           // std::unique_ptr<T> parameter.
#include <stddef.h>                         // size_t parameter.

#include "BitFunnel/Chunks/IChunkWriter.h"  // Base class.


namespace BitFunnel
{
    class IDocument;
    class IFileManager;
    class IShardDefinition;


    //*************************************************************************
    //
    // CopyingChunkWriterFactory
    //
    // Generates CopyingChunkWriters that write chunk files to paths supplied
    // by an IFileManager.
    //
    //*************************************************************************
    class CopyingChunkWriterFactory : public IChunkWriterFactory
    {
    public:
        CopyingChunkWriterFactory(IFileManager& fileManager);


        //
        // IChunkWriterFactory methods.
        //

        virtual std::unique_ptr<IChunkWriter> CreateChunkWriter(size_t index) override;

    protected:
        IFileManager & m_fileManager;
    };


    //*************************************************************************
    //
    // CopyingChunkWriter
    //
    // Writes chunk files to paths supplied by an IFileManager.
    // Primary use case is copying certain documents from one chunk file
    // to another.
    //
    //*************************************************************************
    class CopyingChunkWriter : public IChunkWriter
    {
    public:
        CopyingChunkWriter(IFileManager& fileManager, size_t index);
        virtual ~CopyingChunkWriter();


        //
        // IChunkWriter methods
        //

        virtual void Write(IDocument const & document,
                           char const * start,
                           size_t length) override;

    protected:
        // WARNING: The design of this class relies on the output stream being
        // destructed automatically. The output stream must be destructed in
        // order to close the its underlying file.
        std::unique_ptr<std::ostream> m_output;
    };


    //*************************************************************************
    //
    // AnnotatingChunkWriterFactory
    //
    // Generates AnnotatingChunkWriters that write annotated chunk files to
    // paths supplied by an IFileManager. Each document is annotated with a
    // pseudo term indicating its shard.
    //
    //*************************************************************************
    class AnnotatingChunkWriterFactory : public CopyingChunkWriterFactory
    {
    public:
        AnnotatingChunkWriterFactory(IFileManager& fileManager,
            IShardDefinition const & shardDefinition);


        //
        // IChunkWriterFactory methods.
        //

        virtual std::unique_ptr<IChunkWriter> CreateChunkWriter(size_t index) override;

    private:
        IShardDefinition const & m_shardDefinition;
    };


    //*************************************************************************
    //
    // AnnotatingChunkWriter
    //
    // Writes chunk files to paths supplied by an IFileManager.
    // Each document is annotated with a pseudo-term that indicates the
    // document's shard. The term is added to stream `00` and take the form
    //     SHARD_min_max
    // where `min` and `max` are the shard parameters denoting the range of
    // unique term counts allowed in the shard: [min, max).
    //
    // Primary use case is annotating documents with shard info, in order to
    // facilitate measurement and verification of shard-specific query results.
    //
    //*************************************************************************
    class AnnotatingChunkWriter : public CopyingChunkWriter
    {
    public:
        AnnotatingChunkWriter(IFileManager& fileManager,
                              size_t index,
                              IShardDefinition const & shardDefinition);

        //
        // IChunkWriter methods
        //

        virtual void Write(IDocument const & document,
                           char const * start,
                           size_t length) override;

    private:
        IShardDefinition const & m_shardDefinition;
    };
}
