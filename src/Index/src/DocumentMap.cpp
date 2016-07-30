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
