#pragma once

#include <BitFunnel/Utilities/IInputStream.h>

#include <iosfwd>
#include <utility>


namespace BitFunnel
{
    //*************************************************************************
    //
    // StandardBufferStream is an IInputStream around a std::istream.
    //
    // DESIGN NOTE: This class is designed not to hold any state so that it's
    // possible to create multiple instances (whether at the same time or
    // sequentially) around the same stream and not worry which of the instances
    // is used for a call.
    //*************************************************************************
    class StandardInputStream : public IInputStream
    {
    public:
        // Converts an std::istream reference to IInputStream. The reference
        // must remain valid throughout the lifetime of this object.
        StandardInputStream(std::istream& stream);

        StandardInputStream& operator=(StandardInputStream const &) = delete;

        //
        // IInputStream methods.
        //

        // Attempts to read byteCount bytes from the stream into the
        // destination buffer using std::istream's read() method and returns
        // the actual number of bytes read.
        // The number of bytes read can be less than byteCount only due
        // to a read error or an end-of-file condition.
        virtual size_t Read(char* destination, size_t byteCount) override;

    private:
        std::istream& m_stream;
    };
}
