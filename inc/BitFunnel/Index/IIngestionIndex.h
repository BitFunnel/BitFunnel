#pragma once

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/IFactSet.h"
#include "BitFunnel/IInterface.h"
#include "BitFunnel/DocumentHandle.h"

namespace BitFunnel
{
    class IDocument;
    class IOfflinePerDocumentDataProvider;
    class IOfflinePerDocumentDataIngester;

    //*************************************************************************
    //
    // IIndex is an abstract class or interface of classes that represents the 
    // entire BitFunnel index as viewed by the document ingestion pipeline. 
    // This interface is used by the host of the index to populate the index.
    // Provides methods to add and remove documents, verify if a document 
    // exists in the index, and assert facts on documents in the index.
    //
    // NOTE: caller must call shutdown before destorying the index.
    //
    // Thread safety: all methods are thread safe.
    //
    // TODO: other naming ideas: IIngestionIndex.
    //
    //*************************************************************************
    class IIngestionIndex : public IInterface
    {
    public:
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
        virtual void AssertFact(DocId id, FactHandle fact, bool value) = 0;

        // Returns true if and only if the specified DocId corresponds to a 
        // document currently visible to the query processing system. Returns 
        // false for DocIds that have never been added, DocIds that are 
        // partially ingested, and DocIds that have been deleted.
        virtual bool Contains(DocId id) const = 0;

        // Attempts to allocate a DocumentHandle suitable for ingesting a 
        // document with the specified DocId value and the DDR candidate count.
        // Implementations that don't support Place() will throw.
        //
        // DESIGN NOTE: This method is not intended to be called directly from 
        // the host. Rather, it is called by the implementation of 
        // IDocument::Place(). The reason that IDocument::Place() does not take 
        // a ddrCandidateCount is to decouple the IDocument interface from the 
        // criteria used to actually place documents in shards. If the criteria 
        // should change int the future, say requiring a second parameter, the 
        // signature of IDocument::Place() will remain unchanged while its 
        // implementation will call an IIndex::AllocateDocument() which now 
        // takes one more parameter.
        virtual DocumentHandle AllocateDocument(unsigned ddrCandidateCount) = 0;

        // Returns the size in bytes of the capacity of row tables in the 
        // entire ingestion index.
        virtual unsigned __int64 GetUsedCapacityInBytes() const = 0;

        // Returns the OfflinePerDocumentDataProvider used to access the stored
        // OfflinePerDocumentData.
        virtual IOfflinePerDocumentDataProvider& GetOfflinePerDocumentDataProvider() = 0;

        // Returns the OfflinePerDocumentDataIngester used to store the per-document
        // information.
        // DESIGN NOTE: If ingestion is disabled, the return value will be a nullptr.
        virtual IOfflinePerDocumentDataIngester* GetOfflinePerDocumentDataIngester() = 0;

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
        virtual void OpenNewGroup(GroupId groupId) = 0;

        // Registers an in-recovering group with the manager. If the group already
        // exists, this is a no-op, otherwise, create the accumulator for the group.
        // Note: The client is responsible for calling this function to create a group
        // if the group was missed and going to be recovered. If the missed group
        // is not created using this function, ingestion to this recovered group
        // will be failed.
        // DESIGN NOTE: The function is introduced temporarily only for in-Ram fixup
        // where ingestion may happen on any unsealed accumulators, in 
        // particularly during recovery. Once we have the final SSD version
        // of fixup, this code should be deleted and by default, ingestion can
        // only happen on current accumulator.
        virtual void RegisterRecoveringGroup(GroupId groupId) = 0;

        // Closes the current group, if any.
        virtual void CloseCurrentGroup() = 0;

        // Expires the group with the given id.
        virtual void ExpireGroup(GroupId groupId) = 0;
    };
}
