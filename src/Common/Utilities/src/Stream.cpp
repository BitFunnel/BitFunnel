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


#include <string>

#include "BitFunnel/Stream.h"

namespace BitFunnel
{
    static const char* c_clickName = "click";
    static const char* c_clickExperimentalName = "clickexperimental";
    static const char* c_fullName = "full";
    static const char* c_fullExperimentalName = "fullexperimental";
    static const char* c_metaWordName = "metaword";
    static const char* c_nonBodyName = "nonbody";
    static const char* c_invalidName = "invalid";

    StreamId StringToClassification(const std::string& s)
    {
        if (s.compare(c_clickName) == 0)
        {
            return StreamId::Click;
        }
        else if (s.compare(c_clickExperimentalName) == 0)
        {
            return StreamId::ClickExperimental;
        }
        else if (s.compare(c_fullName) == 0)
        {
            return StreamId::Full;
        }
        else if (s.compare(c_fullExperimentalName) == 0)
        {
            return StreamId::FullExperimental;
        }
        else if (s.compare(c_metaWordName) == 0)
        {
            return StreamId::MetaWord;
        }
        else if (s.compare(c_nonBodyName) == 0)
        {
            return StreamId::NonBody;
        }
        else
        {
            return StreamId::Invalid;
        }
    }


    const char* ClassificationToString(StreamId classification)
    {
        switch (classification)
        {
        case Click:
            return c_clickName;
        case ClickExperimental:
            return c_clickExperimentalName;
        case Full:
            return c_fullName;
        case FullExperimental:
            return c_fullExperimentalName;
        case MetaWord:
            return c_metaWordName;
        case NonBody:
            return c_nonBodyName;
        default:
            return c_invalidName;
        }
    }
}
