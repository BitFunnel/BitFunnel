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
