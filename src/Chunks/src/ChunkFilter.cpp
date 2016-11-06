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
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IIngestor.h"
#include "ChunkFilter.h"
#include "Document.h"


namespace BitFunnel
{
    //std::unique_ptr<IChunkProcessorFactory>
    //    Factories::CreateChunkFilterFactory(
    //        IChunkProcessorFactory & factory,
    //        double randomFraction,
    //        size_t randomSeed,
    //        size_t minPostingCount,
    //        size_t maxPostingCount)
    //{
    //    return std::unique_ptr<IChunkProcessorFactory>(
    //        new ChunkFilterFactory(factory,
    //                               randomFraction,
    //                               randomSeed,
    //                               minPostingCount,
    //                               maxPostingCount));
    //}


    ////*************************************************************************
    ////
    //// ChunkFilterFactory
    ////
    ////*************************************************************************
    //ChunkFilterFactory::ChunkFilterFactory(
    //    IChunkProcessorFactory & factory,
    //    double randomFraction,
    //    size_t randomSeed,
    //    size_t minPostingCount,
    //    size_t maxPostingCount)
    //  : m_factory(factory),
    //    m_randomFraction(randomFraction),
    //    m_randomSeed(randomSeed),
    //    m_minPostingCount(minPostingCount),
    //    m_maxPostingCount(maxPostingCount)
    //{
    //}


    //std::unique_ptr<IChunkProcessor>
    //    ChunkFilterFactory::Create(char const * name, size_t index)
    //{
    //    return std::unique_ptr<IChunkProcessor>(
    //        new ChunkFilter(m_factory.Create(name, index),
    //                        m_randomFraction,
    //                        m_randomSeed,
    //                        m_minPostingCount,
    //                        m_maxPostingCount));
    //}


    //*************************************************************************
    //
    // ChunkFilter
    //
    //*************************************************************************
    ChunkFilter::ChunkFilter(std::unique_ptr<IChunkProcessor> processor,
                             double randomFraction,
                             size_t randomSeed,
                             size_t minPostingCount,
                             size_t maxPostingCount)
      : m_processor(std::move(processor)),
        m_randomFraction(randomFraction),
        m_randomSeed(randomSeed),
        m_minPostingCount(minPostingCount),
        m_maxPostingCount(maxPostingCount)
    {
    }


    void ChunkFilter::OnFileEnter()
    {
        // TODO: Open file stream here?
    }


    void ChunkFilter::OnDocumentEnter(DocId /*id*/)
    {
    }


    void ChunkFilter::OnStreamEnter(Term::StreamId /*id*/)
    {
    }


    void ChunkFilter::OnTerm(char const * /*term*/)
    {
    }


    void ChunkFilter::OnStreamExit()
    {
    }


    void ChunkFilter::OnDocumentExit(IChunkWriter & /*writer*/,
                                     size_t /*bytesRead*/)
    {
        // How do we examine the current document?

        //m_currentDocument->CloseDocument(bytesRead);
        //m_ingestor.Add(m_currentDocument->GetDocId(), *m_currentDocument);
        //if (m_cacheDocuments)
        //{
        //    DocId id = m_currentDocument->GetDocId();
        //    m_ingestor.GetDocumentCache().Add(std::move(m_currentDocument),
        //                                      id);
        //}
        //else
        //{
        //    m_currentDocument.reset(nullptr);
        //}
    }


    void ChunkFilter::OnFileExit(IChunkWriter & /*writer*/)
    {
        // Complete file (need IChunkWriter()).
        // Close stream.
    }
}
