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

#include <iostream>     // TODO: Remove
#include <string>

#include "BitFunnel/Chunks/Factories.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IIndexedIdfTable.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/ISliceBufferAllocator.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/ITermTableCollection.h"
#include "BitFunnel/Mocks/Factories.h"
#include "BitFunnel/Term.h"
#include "LoggerInterfaces/Check.h"
#include "Primes.h"


// TODO: Can we somehow eliminate the requirement for an IConfiguration?
// TODO: Can we somehow eliminate the requirement for an IIndexedIdfTable + TermToText?
// TODO: ChunkManifestIngestor for PrimeFactors corpus?

namespace BitFunnel
{
    //*************************************************************************
    //
    // CreatePrimeFactorsDocument
    //
    //*************************************************************************

    // Constructs a document containing terms corresponding to the prime
    // factors of the supplied DocId.
    //
    // The current implementation does not provide support for phrases.
    //
    // Since this document wasn't constructed from text, the source byte size
    // is computed as the length of a string containing a comma-separated
    // list of the text representation of each factor. For example, the
    // document with DocId=100 would be modeled as the string,
    //     "2,2,5,5"
    // so its source byte size would be 7.
    std::unique_ptr<IDocument>
        Factories::CreatePrimeFactorsDocument(IConfiguration const & config,
                                              DocId docId,
                                              DocId maxDocId,
                                              Term::StreamId streamId)
    {
        const size_t maxPrime = Primes::c_primesBelow10000.back();
        CHECK_LE(docId, maxPrime * maxPrime)
            << "DocId has a prime factor that is not available.";

        CHECK_LE(docId, maxDocId);

        auto document = Factories::CreateDocument(config, docId);
        document->OpenStream(streamId);
        size_t sourceByteSize = 0;

        // Arrange for "0" term row to contain quadwords 0, 1, 2, 3, ...
        size_t quadword = docId >> 6;
        size_t bit = docId % 64;
        if ((quadword & (1ull << bit)) != 0)
        {
            std::string term("0");
            document->AddTerm(term.c_str());
            sourceByteSize += (1 + term.size());
        }

        if (docId != 0)
        {
            for (size_t i = 0; i < Primes::c_primesBelow10000.size(); ++i)
            {
                size_t p = Primes::c_primesBelow10000[i];
                if (p > docId)
                {
                    break;
                }
                else
                {
                    while ((docId % p) == 0)
                    {
                        auto const & term = Primes::c_primesBelow10000Text[i];
                        document->AddTerm(term.c_str());
                        docId /= p;
                        sourceByteSize += (1 + term.size());
                    }
                }
            }

            // Ensure that all prime factors were found. We could miss some
            // prime factors if the square root of the docId is larger than
            // the largest entry in the list of primes. This is more of a logic
            // sanity check since the CHECK_LE at the top of the function should
            // guard against this case.
            CHECK_EQ(docId, 1ull)
                << "DocId value is too large.";
        }

        document->CloseDocument(sourceByteSize);

        return document;
    }


    //*************************************************************************
    //
    // CreatePrimeFactorsTermTable
    //
    //*************************************************************************
    std::unique_ptr<ITermTable>
        Factories::CreatePrimeFactorsTermTable(DocId maxDocId,
                                               Term::StreamId /*streamId*/)
    {
        const Rank rank = 0;
        const RowIndex adhocRowCount = 1;   // Need at least one adhoc row to avoid divide by zero.
        RowIndex explicitRowCount0 = ITermTable::SystemTerm::Count;
        RowIndex explicitRowCount1 = 0;
        RowIndex explicitRowCount2 = 0;

        auto termTable = Factories::CreateTermTable();

        // Term "0"
        termTable->OpenTerm();
        termTable->AddRowId(RowId(rank, explicitRowCount0++));
        termTable->CloseTerm(Term::ComputeRawHash("0"));

        // Term "1"
        termTable->OpenTerm();
        termTable->AddRowId(RowId(rank, explicitRowCount0++));
        termTable->CloseTerm(Term::ComputeRawHash("1"));

        // Terms for primes.
        for (size_t i = 0; i < Primes::c_primesBelow10000.size(); ++i)
        {
            size_t p = Primes::c_primesBelow10000[i];
            if (p > maxDocId)
            {
                break;
            }
            else
            {
                auto text = Primes::c_primesBelow10000Text[i];

                termTable->OpenTerm();
                termTable->AddRowId(RowId(rank, explicitRowCount0++));
                termTable->AddRowId(RowId(rank + 1, explicitRowCount1++));
                termTable->AddRowId(RowId(rank + 2, explicitRowCount2++));
                termTable->CloseTerm(Term::ComputeRawHash(text.c_str()));
            }
        }

        // TODO: Consider adding zero RowIds for AdhocRows. i.e. no calls to
        // CloseAdhocTerm().

        termTable->SetRowCounts(0, explicitRowCount0, adhocRowCount);
        termTable->SetRowCounts(1, explicitRowCount1, adhocRowCount);
        termTable->SetRowCounts(2, explicitRowCount2, adhocRowCount);
        termTable->Seal();

        return termTable;
    }


    //*************************************************************************
    //
    // CreatePrimeFactorsIndex
    //
    //*************************************************************************
    std::unique_ptr<ISimpleIndex>
        Factories::CreatePrimeFactorsIndex(IFileSystem & fileSystem,
                                           DocId maxDocId,
                                           Term::StreamId streamId,
                                           ShardId shardCount)
    {
        // Create special PrimeFactors TermTables containing explicit,
        // private row mappings for terms "0", "1", and the text representation
        // of primes less than or equal to maxDocId.
        auto termTableCollection =
            Factories::CreateTermTableCollection();
        // TODO: don't create the exact same TermTable for each shard?
        for (unsigned i = 0; i < shardCount; ++i)
        {
            auto termTable =
                Factories::CreatePrimeFactorsTermTable(maxDocId, streamId);
            termTableCollection->AddTermTable(std::move(termTable));
        }

        // Need to create our own slice buffer allocator because matcher tests
        // are more comprehensive if there are at least two quadwords in every
        // RowTable row. The ISimpleIndex::CongigureAsMock() method creates an
        // allocator with the absolute minimum block size, which results in a
        // single quadword per row.
        //
        // TODO: Might want to add a check that rows have at least 2 quadwords.
        // Right now the hard-coded blocksize yields 13 quadwords at rank 0,
        // but this could change if the TermTable was configured to use higher
        // ranks.
        size_t blockSize = 20000;
        size_t blockCount = 512;
        auto sliceAllocator =
            Factories::CreateSliceBufferAllocator(blockSize,
                                                  blockCount);

        auto index = Factories::CreateSimpleIndex(fileSystem);
        index->SetTermTableCollection(std::move(termTableCollection));
        index->SetSliceBufferAllocator(std::move(sliceAllocator));

        std::stringstream shardText;
        for (unsigned i = 0; i < shardCount-1; ++i)
        {
            shardText << 2 + i;
            if (i != shardCount - 1)
            {
                shardText << ",";
            }
        }
        auto shardDefinition = Factories::CreateShardDefinition(shardText);
        index->SetShardDefinition(std::move(shardDefinition));

        const Term::GramSize gramSize = 1;
        const bool generateTermToText = false;
        index->ConfigureAsMock(gramSize, generateTermToText);

        index->StartIndex();

        for (DocId docId = 0; docId <= maxDocId; ++docId)
        {
            auto document =
                Factories::CreatePrimeFactorsDocument(
                    index->GetConfiguration(),
                    docId,
                    maxDocId,
                    streamId);
            index->GetIngestor().Add(docId, *document);
        }

        return index;
    }
}
