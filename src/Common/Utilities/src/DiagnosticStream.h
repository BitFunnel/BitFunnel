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

#include <string>                   // Embeds std::string.
#include <vector>                   // Embeds std::vector.

#include "BitFunnel/IDiagnosticStream.h"      // Inherits from IDiagnosticStream.
#include "BitFunnel/NonCopyable.h"            // Inherits from NonCopyable.


namespace BitFunnel
{
    //*************************************************************************
    //
    // DiagnosticStream implements the IDiagnosticStream interface to provice
    // an std::ostream used to output diagnostic information.
    //
    // DiagnosticStream maintains a list of diagnostic keyword prefixes that
    // enable diagnostics for various parts of the system. Code with the
    // ability to emit diagnostic information should use the IsEnabled() method
    // to determine whether to actually write to the diagnostic stream.
    //
    //*************************************************************************
    class DiagnosticStream : public IDiagnosticStream, NonCopyable
    {
    public:
        // Constructs a DiagnosticStream that outputs to the specified stream.
        DiagnosticStream(std::ostream& stream);

        // Adds the diagnostic keyword prefix to the list of prefixes that
        // enable diagnostics.
        void Enable(char const * diagnostic);

        // Removes the diagnostic keyword prefix from the list of prefixes
        // that enable diagnostics.
        void Disable(char const * diagnostic);

        // Returns true if the diagnostic keyword begins with one of the
        // prefixes.
        bool IsEnabled(char const * diagnostic) const;

        // Returns a reference to the stream used for diagnostic output.
        std::ostream& GetStream();

    private:
        std::ostream& m_stream;

        std::vector<std::string> m_enabled;
    };
}
