#pragma once

#include <atomic>
//#include <iosfwd>
//#include <vector>
//
#include "BitFunnel/Index/DocumentHandle.h"     // For DocIndex.
#include "BitFunnel/NonCopyable.h"              // Inherits from NonCopyable.
//#include "BitFunnel/BitFunnelTypes.h"                // DocIndex is a member variable.
//#include "BitFunnel/ThreadsafeCounter.h"             // Member variable.
//#include "Mutex.h"                                   // Member variable.

namespace BitFunnel
{
    //class DocTableDescriptor;
    //class RowTableDescriptor;
    class Shard;

    //*************************************************************************
    //
    // Slice represents a portion of the shard in the BitFunnel index.
    // All Slices within the same Shard have the same capacity. 
    //
    // A Slice allocates a buffer for its data using an ISliceBufferAllocator 
    // provided by its parent Shard. All slice buffers within a single Shard 
    // have the same size and layout. The buffer contains data for DocTable and
    // RowTables which are stored at offsets provided by the Shard's DocTable
    // and RowTable descriptors. The layout and offsets of the DocTable, the 
    // RowTables, and other data structure in a slice buffer is the same for
    // all Slices within a single Shard. In addition, the layout and offset of 
    // the DocTable is the same for all Slices in all Shards in the Index. 
    //
    // DESIGN NOTE : Enforcing a single layout for DocTables across the entire 
    // Index allows us to generate a single scoring plan that runs across all 
    // Shards.
    //
    // For RowTable operations on the slice, one would call methods on 
    // RowTableDescriptor and pass the slice buffer. Similarly for DocTable
    // operations, one would use DocTableDescriptor and the same slice buffer.
    //
    // The main function of the Slice is to allocate document indexes during 
    // document ingestion. Documents may be added to the Slice up to their 
    // capacity. A slice can be serialized to disk after it has been fully 
    // ingested. "Fully ingested" means that all of its columns have been 
    // allocated, and there are no uncommitted documents in progress. A slice
    // indicates that it is fully ingested by returning true from the Commit()
    // method and the thread that gets it is responsible of scheduling this 
    // slice for backup.
    //
    // Documents can be expired from the Slice. When all documents of the slice
    // are expired, the slice can be recycled (removed) from the index. The 
    // slice indicates that it is fully expired by returning true from its 
    // Expire() method and the thread that gets it is responsible of 
    // decrementing the reference count on the Slice.
    //
    // DESIGN NOTE: To prevent destroying a Slice by recycler, the following 
    // synchronization mechanisms are used
    // 1. Query threads use Token mechanism that guarantees availability of 
    //    the Slice for the entire duration of the query thread execution.
    //    Token mechanism is used on the fast code path where the Slice is 
    //    being held for short time typically controlled by a fixed timeout
    //    value.
    // 2. Recycler and Backup use reference count on the Slice. When a Slice is
    //    constructed, its reference count is 1. Backup may take a Slice for 
    //    serializing to disk and increments the ref count by 1 during 
    //    serialization, and decrements by 1 when done. When all of the 
    //    documents in a Slice are expired, its reference count is decremented
    //    by 1. When ref count reaches 0, the Slice is schedule for recycling.
    //    This mechanism is used for entities which need to hold on to the 
    //    Slice for unpredictably longer periods of time.
    // Slice exposes static methods for ref count operations. Static methods 
    // are chosen to avoid a suicide pattern that arise when a Slice may be
    // immediately destroyed on another thread.
    //
    // DESIGN NOTE: While slice buffer is allocated from the specific allocator,
    // the Slice itself is allocated from heap and places a pointer to itself 
    // at the very end of the slice buffer. This allows parties which operate 
    // mostly on slice buffer to get access to the Slice if needed. Slice 
    // pointer is placed in the end of the buffer in order to make the DocTable
    // to start at the same wellknown offset in all Shards. An offset of 0 is
    // a convinient value which is easy to use in Jit implementation of the 
    // scoring engine/DocTable access.
    // 
    // Layout of the slice buffer looks like the following:
    //
    // DocTable data
    // <padding>
    // RowTable0 data
    // <padding>
    // ... (RowTables for other ranks which have rows)
    // RowTable6 data
    // <padding>
    // Slice* (stored in the last 8 bytes of the slice buffer).
    //
    //*************************************************************************
    class Slice : private NonCopyable
    {
    public:
        // Identifier for a slice.
        //typedef unsigned Id;

        // Expected alignment of the slice buffer.
        // DocTable is placed first in the slice, so the alignment requirement
        // is dictated by the DocTable alignment.
        //static const size_t c_bufferByteAlignment = c_docTableByteAlignment;

        // Creates a slice that belogs to a given Shard.
        // Allocates a slice buffer using the allocator from the Shard.
        // Stores pointer to the buffer in m_sliceBuffer.
        Slice(Shard& shard);

        // Creates a slice from its serialized representation from an input 
        // stream. Verifies that the Slice is compatible with the one in the 
        // stream by comparing Shard's RowTableDescriptor and 
        // DocTableDescriptor with copies read from the stream. Throws if the
        // descriptors are not compatible.
        //Slice(Shard& shard, std::istream& input);

        // Releases all heap-allocated data blobs, returns the slice buffer 
        // back to its allocator and destroys the Slice.
        //~Slice();

        // Returns the slice buffer associated with this Slice. Slice buffer
        // is allocated by the Slice using ISliceBufferAllocator from its 
        // parent Shard.
        //void* GetSliceBuffer() const;

        // Returns the shard which owns this slice. 
        // DESIGN NOTE: Shard is required to get access to shared objects at either
        // a Shard level or Index level (e.g. Recycler, backup system etc.)
        Shard& GetShard() const;

        // Returns the RowTable or DocTable descriptors from the parent Shard.
        //DocTableDescriptor const & GetDocTable() const;
        //RowTableDescriptor const & GetRowTable(Rank rank) const;

        // Serializes the slice to a given output stream. Only slices that are
        // full (all columns are allocated and committed) may be serialized.
        // Thread safe with respect to concurrent calls to const methods. 
        //void Write(std::ostream& output) const;

        //
        // Document allocation methods.
        //

        // Attempts to allocate a DocIndex. If Slice is not full, this method 
        // returns true with index set to the allocated DocIndex. Otherwise 
        // this method returns false.
        // Thread safe.
        //
        // Implementation:
        // with (m_docIndexLock)
        //   if (m_unallocated == 0) return false;
        //   m_unallocatedCount--
        //   m_commitPendingCount++
        //   return true
        bool TryAllocateDocument(DocIndex& index);

        // Makes document visible to the matcher. May only be called once per 
        // DocIndex value. Returns true if this was the last document in this 
        // slice to commit, in which case the caller is responsible of 
        // scheduling the Slice for backup. Returns false otherwise.
        // Thread safe.
        //
        // Implementation:
        // with (m_docIndexLock)
        //   LogAssert(m_commitPendingCount > 0)
        //   --m_commitPendingCount;
        //   return (m_unallocated + m_commitPending) == 0;
        //bool CommitDocument(DocIndex index);

        // Hides document from future matching operations. May only be called 
        // once per DocIndex value.
        // DocIndex value must have been successfully allocated by TryAllocateDocument().
        // Returns true if the entire capacity of the Slice is now expired, in 
        // which case the caller is responsible of recycling the Slice. Returns 
        // false otherwise.
        // Thread safe.
        //
        // Implementation:
        // with (m_docIndexLock)
        //   m_expiredCount++;
        //   return m_expiredCount == m_capacity.
        //bool ExpireDocument(DocIndex index);

        // Returns true if the Slice is fully expired, meaning that all of its
        // documents are expired. In this case the Slice can be removed from 
        // the index.
        // TODO: this is exposed in order to verify that only fully expired 
        // Slices are scheduled for recycling. Think if this is needed at all.
        //bool IsExpired() const;

        // Extracts Slice information from the buffer where its data is stored.
        // Slice places a pointer to itself at the offset which is controlled 
        // by Shard.
        //static Slice* GetSliceFromBuffer(void* sliceBuffer,
        //                                 ptrdiff_t slicePtrOffset);

        // Operations on the reference count of the Slice. If the call to
        // DecrementRefCount results in the new value reaching 0, then the
        // Slice is scheduled for recycling.
        // DESIGN NOTE: these methods are made static to avoid a suicide
        // pattern when a call to DecrementRefCount results in the Slice
        // being immediately destroyed by the recycler from another thread
        // while the current thread is still in the Slice code. Technically,
        // IncrementRefCount may be a regular method, but keeping it statis for
        // symmetry.
        //static void IncrementRefCount(Slice* slice);
        //static void DecrementRefCount(Slice* slice);

    private:

        // Initializes the slice buffer and places the pointer to the Slice in the end of the SliceBuffer.
        //void Initialize();

        // Returns a reference to the Slice pointer which is placed inside a sliceBuffer.
        //static Slice*& GetSlicePointer(void* sliceBuffer, ptrdiff_t slicePtrOffset);

        // Shard which owns this slice.
        Shard& m_shard;

        std::atomic<DocIndex> m_temporaryNextDocIndex;

        // Capacity of the slice.
        //const DocIndex m_capacity;

        // Lock protecting operations on DocIndex members below to guarantee 
        // atomic state changes that depend on these values.
        // DESIGN NOTE: made mutable in order to be able to acquire a lock from
        // const methods.
        //mutable Mutex m_docIndexLock;

        // Reference count of the Slice. Initially Slice is created with one 
        // reference. Slice taken for a backup increases its reference count 
        // by one for the duration of the backup writing and then is decreased
        // by 1. When a Slice is recycled, its reference count is decreased 
        // by 1. When reference count reaches 0, the Slice can be scheduled
        // for recycling.
        //ThreadsafeCounter32 m_refCount;

        // WARNING: The persistence format depends on the order in which the
        // following members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the Write() method.

        // Pointer to a buffer of data for RowTables and DocTable for this 
        // Slice. See the class comment for more details on buffer layout.
        //void* const m_buffer;

        // The number of unallocated DocIndex'es in the slice. When created, 
        // Slice starts with the value of m_capacity in this field and gradually
        // goes down as documents are being ingested.
        //DocIndex m_unallocatedCount;

        // The number of DocIndex'es that have been allocated but not yet 
        // committed by a call to CommitDocument().
        //DocIndex m_commitPendingCount;

        // The number of DocIndex'es that have been expired from the slice. 
        // When this value reaches m_capacity, the slice can be recycled.
        //DocIndex m_expiredCount;
    };
}
