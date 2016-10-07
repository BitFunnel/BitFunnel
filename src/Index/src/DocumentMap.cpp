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

#include <sstream>

#include "BitFunnel/Exceptions.h"
#include "DocumentMap.h"


namespace BitFunnel
{
    void DocumentMap::Add(DocumentHandleInternal handle)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        DocId id = handle.GetDocId();

        // Verify that this DocId hasn't been added previously.
        auto it = m_docIdToDocHandle.find(id);
        if (it != m_docIdToDocHandle.end())
        {
            std::stringstream message;
            message << "Ingestor::Add(): DocId " << id << " has already been added.";

            RecoverableError error(message.str());
        }

        m_docIdToDocHandle.insert(std::make_pair(id, handle));
    }


    DocumentHandleInternal DocumentMap::Find(DocId id, bool& isFound) const
    {
        std::lock_guard<std::mutex> lock(m_lock);

        DocumentHandleInternal handle;

        auto it = m_docIdToDocHandle.find(id);
        if (it == m_docIdToDocHandle.end())
        {
            isFound = false;
        }
        else
        {
            isFound = true;
            handle = (*it).second;
        }

        return handle;
    }


    bool DocumentMap::Delete(DocId id)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        DocumentHandleInternal handle;

        auto it = m_docIdToDocHandle.find(id);
        bool found = (it != m_docIdToDocHandle.end());
        if (found)
        {
            m_docIdToDocHandle.erase(it);
        }

        return found;
    }
}
