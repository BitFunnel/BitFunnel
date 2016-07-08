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
