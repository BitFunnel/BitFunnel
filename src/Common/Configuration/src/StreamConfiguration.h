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

#include <array>                                            // Embedded.
#include <unordered_map>                                    // Embedded.
#include <vector>                                           // Embedded.

#include "BitFunnel/Configuration/IStreamConfiguration.h"   // Base class.


namespace BitFunnel
{
    class StreamConfiguration : public IStreamConfiguration
    {
    public:
        // This class uses a simple table to get O(1) lookups.
        // The table is indexed by Term::StreamId. Therefore, we
        // must limit the maximum Term::StreamId value.
        static const Term::StreamId c_maxStreamIdValue = 255;

        StreamConfiguration();
        StreamConfiguration(std::istream& input);

        //
        // IStreamConfiguration methods.
        //

        virtual Term::StreamId GetStreamId(char const * name) const override;

        virtual std::vector<Term::StreamId> const &
            GetIndexStreamIds(Term::StreamId documentStreamId) const override;

        virtual std::vector<Term::StreamId> const &
            GetDocumentStreamIds(Term::StreamId indexStreamId) const override;

        virtual void AddMapping(
            char const * indexStreamName,
            std::vector<Term::StreamId> const &documentStreamdIds) override;

        virtual void Write(std::ostream& output) const override;

    private:
        void EnsureNoDuplicates(std::vector<Term::StreamId> const & ids) const;

        std::unordered_map<std::string, Term::StreamId> m_textToStreamId;

        typedef
            std::array<std::vector<Term::StreamId>, c_maxStreamIdValue + 1>
            StreamIdMapping;

        StreamIdMapping m_documentToIndex;
        StreamIdMapping m_indexToDocument;
    };
}
