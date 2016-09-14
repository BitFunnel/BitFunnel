#pragma once

#include <ostream>      // std::ostream return value.


namespace BitFunnel
{
    //*************************************************************************
    //
    // IDiagnosticStream is an abstract base class or interface for objects
    // that provide an std::ostream used to output diagnostic information.
    //
    // IDiagnosticStream maintains a list of diagnostic keyword prefixes that
    // enable diagnostics for various parts of the system. Code with the
    // ability to emit diagnostic information should use the IsEnabled() method
    // to determine whether to actually write to the diagnostic stream.
    //
    //*************************************************************************
    class IDiagnosticStream
    {
    public:
        virtual ~IDiagnosticStream() {};

        // Adds the diagnostic keyword prefix to the list of prefixes that
        // enable diagnostics.
        virtual void Enable(char const * prefix) = 0;

        // Removes the diagnostic keyword prefix from the list of prefixes
        // that enable diagnostics.
        virtual void Disable(char const * prefix) = 0;

        // Returns true if the diagnostic keyword begins with one of the
        // prefixes.
        virtual bool IsEnabled(char const * diagnostic) const = 0;

        // Returns a reference to the stream used for diagnostic output.
        virtual std::ostream& GetStream() = 0;
    };
}
