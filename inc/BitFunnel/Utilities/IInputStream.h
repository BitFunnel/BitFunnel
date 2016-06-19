#pragma once

namespace BitFunnel
{
    //*************************************************************************
    // IInputStream is an interface for classes that read data from an input
    // stream.
    //
    // DESIGN NOTE: For simplicity, the interface only has a method to read the
    // data for the stream and has no methods to check whether end of file
    // condition has occurred, whether a read error has occurred or to clear
    // the read error, as they would not be used by clients.
    //
    //*************************************************************************
    class IInputStream
    {
    public:
        virtual ~IInputStream() {}

        // Attempts to read byteCount bytes from the stream into the
        // destination buffer and returns the number of bytes actually read.
        // The number of bytes read will be less than requested only in case of
        // a read error or an end-of-file condition.
        //
        // The implementation must not throw on reading over end of file,
        // it should return number of bytes read < byteCount.
        virtual size_t Read(char* destination, size_t byteCount) = 0;
    };
}
