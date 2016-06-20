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

#include <memory>                       // std::unqiue_ptr member.
#include <vector>                       // std::vector member.

#include "BitFunnel/Index/IDocument.h"  // std::unique_ptr<IDocument>.
#include "BitFunnel/NonCopyable.h"      // Inherits from NonCopyable.
#include "ChunkReader.h"                // Inherits from ChunkReader::IEvents.


namespace BitFunnel
{
    class IDocumentFactory
    {
    };
}


namespace BitFunnel
{
    class IConfiguration;
    class IIngestor;

    // DESIGN NOTE: Consider adding a document factory parameter to the
    // constructor.
    class ChunkIngestor : public NonCopyable, public ChunkReader::IEvents
    {
    public:
        // TODO: We need to implement IDocumentFactory before this make sense.
        // ChunkIngestor(std::string const & filePath, IIndex& index,
        //               IDocumentFactory& factory);
        ChunkIngestor(std::vector<char> const& chunkData,
                      IConfiguration const & configuration,
                      IIngestor& ingestor);

        //
        // ChunkReader::IEvents methods.
        //
        virtual void OnFileEnter() override;
        virtual void OnDocumentEnter(DocId id) override;
        virtual void OnStreamEnter(char const * name) override;
        virtual void OnTerm(char const * term) override;
        virtual void OnStreamExit() override;
        virtual void OnDocumentExit() override;
        virtual void OnFileExit() override;

    private:
        //
        // Constructor parameters
        //
        IConfiguration const & m_config;
        IIngestor& m_ingestor;

        //
        // Other members
        //
        std::vector<char> const& m_chunkData;
        std::unique_ptr<IDocument> m_currentDocument;
    };
}
