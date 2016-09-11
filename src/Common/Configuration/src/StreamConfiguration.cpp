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

#include "BitFunnel/Exceptions.h"
#include "StreamConfiguration.h"


namespace BitFunnel
{
    StreamConfiguration::StreamConfiguration()
    {
    }


    StreamConfiguration::StreamConfiguration(std::istream& /*input*/)
    {
        // TODO: Implement
    }


    Term::StreamId StreamConfiguration::GetStreamId(char const * name) const
    {
        auto it = m_textToStreamId.find(name);
        if (it == m_textToStreamId.end())
        {
            RecoverableError error("StreamConfiguration: stream not found.");
            throw error;
        }
        else
        {
            return it->second;
        }
    }


    std::vector<Term::StreamId> const & StreamConfiguration::GetIndexStreamIds(
        Term::StreamId documentStreamId) const
    {
        if (documentStreamId >= m_documentToIndex.size())
        {
            RecoverableError error("StreamConfiguration: StreamId out of range.");
            throw error;
        }
        else
        {
            return m_documentToIndex[documentStreamId];
        }
    }


    std::vector<Term::StreamId> const &
        StreamConfiguration::GetDocumentStreamIds(
            Term::StreamId indexStreamId) const
    {
        if (indexStreamId >= m_indexToDocument.size())
        {
            RecoverableError error("StreamConfiguration: StreamId out of range.");
            throw error;
        }
        else
        {
            return m_indexToDocument[indexStreamId];
        }

    }


    void StreamConfiguration::AddMapping(
        char const * indexStreamName,
        std::vector<Term::StreamId> const & documentStreamdIds)
    {
        // Ensure that adding a new stream name won't cause us to define a new
        // StreamId whose value would be too large to use as an index into 
        // m_indexToDocument.
        if (m_textToStreamId.size() > c_maxStreamIdValue)
        {
            RecoverableError error("StreamConfiguration: Too many stream names.");
            throw error;
        }

        // Ensure that the list of document StreamIds associated with the new
        // index stream is free of duplicates.
        EnsureNoDuplicates(documentStreamdIds);

        // Ensure the new stream name doesn't already exist.
        auto it = m_textToStreamId.find(indexStreamName);
        if (it != m_textToStreamId.end())
        {
            RecoverableError error("StreamConfiguration: Stream name in use.");
            throw error;
        }
        else
        {
            // Assign an index StreamId.
            Term::StreamId indexStreamId =
                static_cast<Term::StreamId>(m_textToStreamId.size());

            // Add index stream name to hash table.
            m_textToStreamId.insert(
                std::make_pair(std::string(indexStreamName),
                               indexStreamId));

            // Add mapping from indexStreamId to documentStreamIds.
            m_indexToDocument[indexStreamId] = documentStreamdIds;

            // Add mapping from documentStreamId to indexStreamIds.
            for (auto id : documentStreamdIds)
            {
                m_documentToIndex[id].push_back(indexStreamId);
            }
        }
    }


    void StreamConfiguration::Write(std::ostream& /*output*/) const
    {
        // TODO: Implement
        throw NotImplemented();
    }


    void StreamConfiguration::EnsureNoDuplicates(std::vector<Term::StreamId> const & /*ids*/) const
    {
        // TODO: implement this check.
    }
}
