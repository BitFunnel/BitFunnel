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

#include <iostream>     // TODO: Remove this temporary header.
#include <memory>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "DocumentHandleInternal.h"
#include "Ingestor.h"


namespace BitFunnel
{
    std::unique_ptr<IIngestor> Factories::CreateIngestor()
    {
        return std::unique_ptr<IIngestor>(new Ingestor());
    }


    Ingestor::Ingestor()
        : m_termCount(0),
          m_documentCount(0)
    {
        // Initialize histogram and frequency tables here.
        m_shards.push_back(std::unique_ptr<Shard>(new Shard(*this, 123)));
    }


    void Ingestor::PrintStatistics() const
    {
        std::cout << "Document count: " << m_documentCount << std::endl;
        std::cout << "Term count: " << m_termCount << std::endl;

        for (auto it = m_shards.begin(); it != m_shards.end(); ++it)
        {
            (*it)->TemporaryPrintFrequencies(std::cout);
        }
    }



    void Ingestor::Add(DocId /*id*/, IDocument const & document)
    {
        ++m_documentCount;
        m_termCount += document.GetPostingCount();

        DocumentHandleInternal handle = m_shards[0]->AllocateDocument();
        document.Ingest(handle);
    }


    bool Ingestor::Delete(DocId /*id*/)
    {
        throw NotImplemented();
    }


    void Ingestor::AssertFact(DocId /*id*/, FactHandle /*fact*/, bool /*value*/)
    {
        throw NotImplemented();
    }


    bool Ingestor::Contains(DocId /*id*/) const
    {
        throw NotImplemented();
    }


    size_t Ingestor::GetUsedCapacityInBytes() const
    {
        throw NotImplemented();
    }


    void Ingestor::Shutdown()
    {
        throw NotImplemented();
    }


    void Ingestor::OpenGroup(GroupId /*groupId*/)
    {
        throw NotImplemented();
    }


    void Ingestor::CloseGroup()
    {
        throw NotImplemented();
    }


    void Ingestor::ExpireGroup(GroupId /*groupId*/)
    {
        throw NotImplemented();
    }
}
