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

#include <iosfwd>                   // std::ostream& parameter.
#include <vector>                   // std::vector return value.

#include "BitFunnel/IInterface.h"   // Base class.
#include "BitFunnel/Term.h"         // Term::StreamId return value.


namespace BitFunnel
{
    //*************************************************************************
    //
    // IStreamConfiguration
    //
    // Abstract base class or interface for classes implementing a many-to-many
    // mapping between document StreamIds and index StreamIds.
    //
    //*************************************************************************
    class IStreamConfiguration : public IInterface
    {
    public:
        // Returns the index StreamId associated with a specific name.
        // Throws if there is no associated StreamId.
        virtual Term::StreamId GetStreamId(char const * name) const = 0;

        // Returns a vector of index StreamIds associated with a particular
        // document StreamId.
        virtual std::vector<Term::StreamId> const &
            GetIndexStreamIds(Term::StreamId documentStreamId) const = 0;

        // Returns a vector of document StreamIds associated with a particular
        // index StreamId.
        virtual std::vector<Term::StreamId> const &
            GetDocumentStreamIds(Term::StreamId indexStreamId) const = 0;

        // Defines a new index StreamId for indexStreamName and maps it to the
        // vector of document StreamIds.
        virtual void AddMapping(char const * indexStreamName,
                                std::vector<Term::StreamId> const & documentStreamdIds) = 0;

        // Writes the mapping to a stream. The file format is one mapping
        // per line, where each mapping consists of a stream name, followed
        // by a comma-separated list of document StreamId values, e.g.
        // "title,0,1\nbody,2,3\n".
        virtual void Write(std::ostream& output) const = 0;
    };
}
