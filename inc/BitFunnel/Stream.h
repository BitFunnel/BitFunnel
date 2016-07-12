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

#include <string>

// TODO: remove stream types we don't have outside of Bing.

namespace BitFunnel
{
    enum StreamId {Click = 0,
                   ClickExperimental = 1,
                   Full = 2,
                   FullExperimental = 3,
                   MetaWord = 4,
                   NonBody = 5,
                   ClassificationCount = 6,
                   Invalid = -1};

    // Converts a string with a classification name into a
    // Stream::Classification. Returns Invalid if string does not
    // correspond to a known Classification. See Stream.cpp for the string
    // constants corresponding to each Classification. The main scenario for
    // this function is reading in configuration files.
    StreamId StringToClassification(const std::string& s);

    // Returns the textual representation of a Stream::Classification. The
    // return string is owned by ClassificationToString and should not be
    // deleted by the caller.
    const char* ClassificationToString(StreamId classification);
}
