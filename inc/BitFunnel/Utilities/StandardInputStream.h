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
