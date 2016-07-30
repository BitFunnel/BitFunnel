#pragma once

#include <mutex>                        // std::mutex member.
#include <unordered_map>                // std::unordered_map member.

#include "BitFunnel/BitFunnelTypes.h"   // For DocId parameter.
#include "BitFunnel/NonCopyable.h"      // Base class.
#include "DocumentHandleInternal.h"     // DocHandleInternal template parameter.


namespace BitFunnel
{
    class DocumentMap : NonCopyable
    {
    public:
        // Adds a new (DocId, DocumentHandleInternal) pair to the map. DocId is
        // obtained from DocumentHandleInternal::GetDocId(). Throws if the map 
        // already contains an entry for a given DocId.        
        void Add(DocumentHandleInternal value);

        // Attempts to find the DocumentHandleInternal corresponding to the 
        // specified DocId value. If such a DocumentHandleInternal exists, a 
        // copy will be returned after setting isFound to true. Otherwise 
        // isFound will be set to false and the return value is undefined.
        // 
        // DESIGN NOTE: Returning a copy of the item instead of the reference 
        // is done to avoid the multithreading problem that exists when another 
        // thread deletes or modifies this item while we are holding the 
        // reference.
        DocumentHandleInternal Find(DocId id, bool& isFound) const;

        // Deletes an entry which corresponds to the given DocId. If no such entry
        // exists, the request is ignored and the function returns false.
        // Returns true otherwise.
        bool Delete(DocId id);

    private:
        // Lock protecting operations on m_docIdToHandle.
        // Made mutable to allow using it from const functions.
        mutable std::mutex m_lock;

        std::unordered_map<DocId, DocumentHandleInternal> m_docIdToDocHandle;
    };
}
