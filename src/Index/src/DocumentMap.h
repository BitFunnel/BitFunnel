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
