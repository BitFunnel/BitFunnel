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
