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

#include <iosfwd>               // std::istream parameter.
#include <memory>               // std::unique_ptr return type.
#include <vector>               // std::vector parameter.

#include "BitFunnel/Term.h"                 // Term::IdfX10 parameter.
#include "BitFunnel/BitFunnelTypes.h"       // DocIndex parameter.
#include "BitFunnel/Index/DocumentHandle.h" // DocumentHandle return value.


namespace BitFunnel
{
    class IChunkManifestIngestor;
    class IConfiguration;
    class IDocument;
    class IDocumentDataSchema;
    class IDocumentHistogram;
    class IDocumentFrequencyTable;
    class IFactSet;
    class IFileManager;
    class IFileSystem;
    class IIngestor;
    class IRecycler;
    class IShardCostFunction;
    class IShardDefinition;
    class ISimpleIndex;
    class ISliceBufferAllocator;
    class ITermTable;
    class ITermTableCollection;
    class ITermTableBuilder;
    class ITermTableCollection;
    class ITermToText;
    class ITermTreatment;
    class ITermTreatmentFactory;
    class Slice;

    namespace Factories
    {
        void AnalyzeRowTables(ISimpleIndex const & index,
                              char const * outDir);

        void CreateCorrelate(ISimpleIndex const & index,
                             char const * outDir,
                             std::vector<std::string> const & terms);

        std::unique_ptr<IConfiguration>
            CreateConfiguration(size_t maxGramSize,
                                bool keepTermText,
                                IFactSet const & facts);

        std::unique_ptr<IDocumentDataSchema> CreateDocumentDataSchema();

        std::unique_ptr<IDocumentFrequencyTable>
            CreateDocumentFrequencyTable(std::istream& input);

        DocumentHandle CreateDocumentHandle(Slice * sliceBuffer, DocIndex index);
        DocumentHandle CreateDocumentHandle(void * sliceBuffer, DocIndex index);

        std::unique_ptr<IDocumentHistogram> CreateDocumentHistogram(std::istream& input);

        // TODO: Remove this temporary function.
        void * GetSliceBuffer(Slice * slice);

        std::unique_ptr<IFactSet> CreateFactSet();

        std::unique_ptr<IIngestor>
            CreateIngestor(IDocumentDataSchema const & docDataSchema,
                           IRecycler& recycler,
                           ITermTableCollection const & termTables,
                           IShardDefinition const & shardDefinition,
                           ISliceBufferAllocator& sliceBufferAllocator);

        std::unique_ptr<IRecycler> CreateRecycler();

        std::unique_ptr<IShardCostFunction>
            CreateShardCostFunction(IDocumentHistogram const & histogram,
                                    double shardOverhead,
                                    size_t minShardCapacity,
                                    Rank maxRankInUse);

        std::unique_ptr<ISimpleIndex> CreateSimpleIndex(IFileSystem& fileSystem);

        std::unique_ptr<ISliceBufferAllocator>
            CreateSliceBufferAllocator(size_t blockSize, size_t blockCount);

        std::unique_ptr<ITermTable> CreateTermTable();
        std::unique_ptr<ITermTable> CreateTermTable(std::istream & input);

        std::unique_ptr<ITermTableBuilder>
            CreateTermTableBuilder(double density,
                                   double adhocFrequency,
                                   ITermTreatment const & treatment,
                                   IDocumentFrequencyTable const & terms,
                                   IFactSet const & facts,
                                   ITermTable & termTable);

        std::unique_ptr<ITermTableCollection>
            CreateTermTableCollection();
        std::unique_ptr<ITermTableCollection>
            CreateTermTableCollection(ShardId shardCount);
        std::unique_ptr<ITermTableCollection>
            CreateTermTableCollection(IFileManager & fileManager,
                                      ShardId shardCount);

        std::unique_ptr<ITermTreatmentFactory> CreateTreatmentFactory();

        std::unique_ptr<ITermToText> CreateTermToText(std::istream & input);
        std::unique_ptr<ITermToText> CreateTermToText();
    }
}
