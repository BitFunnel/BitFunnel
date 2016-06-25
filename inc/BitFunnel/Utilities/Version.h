#pragma once

#include <istream>
#include <ostream>

#include "BitFunnel/Utilities/IInputStream.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // Version
    //
    // Represents a version number. Version numbers have three components:
    //   Major: a mismatch in major version means that the header format has
    //          changed and a newer version of the FileHeader class is needed
    //          in order to correctly parse the header.
    //   Middle: a mismatch in the middle version means that the data portion
    //           of the file format has changed.
    //   Minor: a mismatch in the minor version means the data portion of the
    //          file is compatible, but that bug fixes might have changed
    //          the contents in some way.
    //
    // DESIGN NOTE: The Version class does not specify a versioning policy
    // for the system. Users could choose, for example to use a single, 
    // system-wide version, or at the other end of the spectrum, a set of
    // supported versions for each major component (e.g RowTable supports 1.1.XX
    // today and also 1.0.XX for backwards compatability).
    //*************************************************************************
    class Version
    {
    public:
        // Build-time constructor.
        Version(int major, int middle, int minor);

        // Read-time constructor taking istream.
        Version(std::istream& in);

        // Read-time constructor taking IInputStream.
        Version(IInputStream& in);

        // Writes the version object to a stream.
        void Write(std::ostream& out) const;

        // Returns true iff the major and middle version numbers are equal to their counterparts in other.
        bool IsCompatibleWith(const Version& other) const;

        // Return the Major, Middle, or Minor version number.
        int VersionMajor() const;
        int VersionMiddle() const;
        int VersionMinor() const;

        // Generate new Version by incrementing the Major, Middle, or Minor version number.
        Version IncrementMajor() const;     // Increment Major, set Middle and Minor to zero.
        Version IncrementMiddle() const;    // Increment Middle, set Minor to zero.
        Version IncrementMinor() const;     // Increment Minor only.

    private:
        int m_versionMajor;     // Header format not compatible
        int m_versionMiddle;    // File format not compatible
        int m_versionMinor;     // Compatible, but bugs fixes
    };
}
