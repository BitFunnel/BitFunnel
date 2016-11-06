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

#include <iosfwd>   // std::ostream parameter.

#include "BitFunnel/Chunks/IChunkProcessor.h"   // Base class.
#include "BitFunnel/NonCopyable.h"      // Base class.

xxx force compile error so I will read this note.
//Not sure if this is how I want to do this.
//Benefits of copying blob
//  1. Can copy after all callbacks. Otherwise don't know when to start copying.
//  2. Don't have to rigorously test copy. e.g. hexidecimal output case correct.

namespace BitFunnel
{
    class ChunkWriter : public NonCopyable, public IChunkProcessor
    {
    public:
        ChunkWriter(std::ostream& output);

        //
        // IChunkProcessor methods
        //

        virtual void OnFileEnter() override;
        virtual void OnDocumentEnter(DocId id) override;
        virtual void OnStreamEnter(Term::StreamId id) override;
        virtual void OnTerm(char const * term) override;
        virtual void OnStreamExit() override;
        virtual void OnDocumentExit(size_t bytesRead) override;
        virtual void OnFileExit() override;

    private:
        std::ostream& m_output;
    };


    class ChunkFileWriter : public NonCopyable, public IChunkProcessor
    {
    public:
        ChunkFileWriter(IFileSystem & filesystem,
                        std::string const & baseName,
                        std::string const & extension,
                        size_t fileSize);

        //
        // IChunkProcessor methods
        //

        virtual void OnFileEnter() override;
        virtual void OnDocumentEnter(DocId id) override;
        virtual void OnStreamEnter(Term::StreamId id) override;
        virtual void OnTerm(char const * term) override;
        virtual void OnStreamExit() override;
        virtual void OnDocumentExit(size_t bytesRead) override;
        virtual void OnFileExit() override;

    private:
        IFileSystem & m_fileSystem;
        std::string m_baseName;
        std::string m_extension;
        size_t m_fileCount;
        std::auto_ptr<std::ostream> m_output;
        std::auto_ptr<ChunkWriter> m_writer;
    };
}
