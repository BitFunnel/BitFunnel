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

#include <string>

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Term.h"
#include "PrimeFactorsDocument.h"
#include "Primes.h"


// TODO: Can we somehow eliminate the requirement for an IConfiguration?
// TODO: Can we somehow eliminate the requirement for an IIndexedIdfTable + TermToText?
// TODO: ChunkManifestIngestor for PrimeFactors corpus?

namespace BitFunnel
{
    std::unique_ptr<IDocument>
        CreatePrimeFactorsDocument(IConfiguration const & config,
                                   DocId docId,
                                   Term::StreamId streamId)
    {
        auto document = Factories::CreateDocument(config, docId);
        document->OpenStream(streamId);
        document->AddTerm("1");
        size_t sourceByteSize = 1;

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

        //std::string term = std::to_string(docId);
        //document->AddTerm(term.c_str());
        //sourceByteSize += (1 + term.size());

        document->CloseDocument(sourceByteSize);

        return document;
    }


    std::unique_ptr<ITermTable>
        CreatePrimeFactorsTermTable(DocId maxDocId,
                                    Term::StreamId /*streamId*/,
                                    IConfiguration const & /*config*/)
    {
        const ShardId shard = 0;
        const Rank rank = 0;
        const RowIndex adhocRowCount = 1;   // Need at least one adhoc row to avoid divide by zero.
        RowIndex explicitRowCount = 0;

        auto termTable = Factories::CreateTermTable();

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
                termTable->AddRowId(RowId(shard, rank, explicitRowCount++));
                termTable->CloseTerm(Term::ComputeRawHash(text.c_str()));
            }
        }

        // TODO: Consider adding zero RowIds for AdhocRows. i.e. no calls to
        // CloseAdhocTerm().

        termTable->SetRowCounts(0, explicitRowCount, adhocRowCount);
        termTable->Seal();

        return termTable;
    }
}
