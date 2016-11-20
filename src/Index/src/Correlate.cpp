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


#include <ostream>
#include <stack>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Index/Token.h"
#include "Correlate.h"
#include "CsvTsv/Csv.h"
#include "DocumentHandleInternal.h"
#include "LoggerInterfaces/Check.h"
#include "RowTableAnalyzer.h"
#include "RowTableDescriptor.h"
#include "Shard.h"
#include "Slice.h"
#include "TermToText.h"


namespace BitFunnel
{
    void Factories::Correlate(ISimpleIndex const & index,
                              char const * outDir)
    {
        CHECK_NE(*outDir, '\0')
            << "Output directory not set. ";

        Correlate::Correlate correlate(index);
        // TODO: call methods here.
    }


    Correlate::Correlate(ISimpleIndex const & index)
        : m_index(index)
    {
    }


    void Correlate::CorrelateRows(char const * outDir) const
    {
        auto & fileManager = m_index.GetFileManager();
        auto & ingestor = m_index.GetIngestor();

        // // TODO: Create with factory?
        // TermToText termToText(*fileManager.TermToText().OpenForRead());

        for (ShardId shardId = 0; shardId < ingestor.GetShardCount(); ++shardId)
        {
            auto terms(Factories::CreateDocumentFrequencyTable(
                *fileManager.DocFreqTable(shardId).OpenForRead()));

            auto fileSystem = Factories::CreateFileSystem();
            auto outFileManager =
                Factories::CreateFileManager(outDir,
                                             outDir,
                                             outDir,
                                             *fileSystem);

            // TODO: hoist this read out of loop?
            CorrelateShard(shardId,
                           // termToText,
                           *outFileManager->RowDensities(shardId).OpenForWrite());
        }
    }


    void Correlate::CorrelateShard(
                                   ShardId const & /*shardId*/,
        // ITermToText const & termToText,
                                   std::ostream& /*out*/) const
    {
        // auto & fileManager = m_index.GetFileManager();
    }

}
