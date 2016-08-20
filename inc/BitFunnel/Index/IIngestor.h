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

#include <iosfwd>                               //std::istream& parameter.

#include "BitFunnel/IInterface.h"               // IIngestor inherits from IInterface.
#include "BitFunnel/Index/IFactSet.h"           // FactHandle parameter.
#include "BitFunnel/Index/DocumentHandle.h"     // DocHandle return value.


namespace BitFunnel
{
    class IDocument;
    class IFileManager;
    class IRecycler;
    class ITokenManager;
    class Shard;
    class TermToText;

    // BITFUNNELTYPES
    // The documents in the BitFunnel index can be grouped into conceptual
    // groups based on the need of the client. For example, a client can choose
    // to group documents based on time stamp. Each group is assigned a GroupId
    // which is a unique identifier for the group that the client can use to
    // manage the index.
    typedef size_t GroupId;


    //*************************************************************************
    //
    // IIngester is an abstract class or interface of classes that
    // represents the entire BitFunnel index as viewed by the document ingestion
    // pipeline. This interface is used by the host of the index to populate the
    // index. Provides methods to add and remove documents, verify if a document
    // exists in the index, and assert facts on documents in the index.
    //
    // NOTE: caller must call Shutdown() before destroying the index.
    //
    // Thread safety: all methods are thread safe.
    //
    //*************************************************************************
    class IIngestor : public IInterface
    {
    public:
        // TODO: Remove this temporary method.
        virtual void PrintStatistics() const = 0;

        // Writes out the following data structions in locations defined by the
        // FileManager:
        //
        //   Per IIngester
        //      DocumentLengthHistogram
        //   Per Shard
        //      CumulativeTermCountd
        //      DocumentFrequencyTable (with term text if termToText provided)
        //      IndexedIdfTable
        virtual void WriteStatistics(IFileManager & fileManager,
                                     TermToText const * termToText) const = 0;

        // Adds a document to the index. Throws if there is no space to add the
        // document which means the system is running at its maximum capacity.
        // The IDocument must implement the Place method which should call
        // IIndex::AllocateDocument, passing the ddrCandidateCount parameter.
        // Throws if the index already contains a document with the same id
        // value.
        virtual void Add(DocId id, IDocument const & document) = 0;

        // Removes a document from serving. The document with the specified id
        // will no longer be returned from the queries. Returns true if the
        // document was successfully removed and false otherwise. False means
        // that a document with this id wasn't added previously or it was
        // already removed.
        //
        // DESIGN NOTE: The system supports deleting a document that has been
        // already deleted in order to simplify bulk deletion of documents,
        // some of which may already have been deleted for other reasons.
        virtual bool Delete(DocId id) = 0;

        // Sets or clears a fact about a document with the given DocId. The
        // FactHandle must have been previously registered in the IFactSet,
        // otherwise the function throws.
        // TODO: Update this comment to explain which IFactSet is referred
        // to above.
        virtual void AssertFact(DocId id, FactHandle fact, bool value) = 0;

        // Returns true if and only if the specified DocId corresponds to a
        // document currently visible to the query processing system. Returns
        // false for DocIds that have never been added, DocIds that are
        // partially ingested, and DocIds that have been deleted.
        virtual bool Contains(DocId id) const = 0;

        // This method exists so that IngestAndQuery REPL can display bits for
        // various rows. Not sure it is needed in the long run.
        virtual DocumentHandle GetHandle(DocId id) const = 0;

        // Returns the size in bytes of the capacity of row tables in the
        // entire ingestion index.
        virtual size_t GetUsedCapacityInBytes() const = 0;

        // Returns the total number of bytes in the source representation of
        // all IDocuments ingested so far.
        virtual size_t GetTotalSouceBytesIngested() const = 0;

        // Returns a number of Shards and a Shard with the given ShardId.
        virtual size_t GetShardCount() const = 0;
        virtual Shard& GetShard(size_t shard) const = 0;

        virtual IRecycler& GetRecycler() const = 0;

        virtual ITokenManager& GetTokenManager() const = 0;

        // Shuts down the index and releases resources allocated to it.
        virtual void Shutdown() = 0;

        //
        // Group management functions.
        //
        // A group is a sequence of documents that were ingested after the opening
        // of the group and its sealing. Once the group is closed, it is considered
        // immutable. When expiring a group, all the data related to documents
        // that were part of this group will be deleted.
        //

        // Opens a new group and assigns it the given group id.
        //    - All future addition operations are done in this new group.
        //    - The previous group is closed. A closed group cannot be reopened or
        //      modified.
        virtual void OpenGroup(GroupId groupId) = 0;

        // Closes the current group, if any.
        virtual void CloseGroup() = 0;

        // Expires the group with the given id.
        virtual void ExpireGroup(GroupId groupId) = 0;
    };
}
