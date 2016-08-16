#pragma once

#include <string>               // std::string uses as a parameter.


namespace BitFunnel
{
    // The classification of a term in a query
    enum Classification { Invalid =-1, Full = 0, NonBody, MetaWord,
                          ClickBoost, ClassificationCount };

    // Converts a string with a classification name into a Classification.
    // Returns Classification::InvalidClassification if string does not
    // correspond to a known Classification.
    Classification StringToClassification(const std::string& classification);

    // Returns the textual representation of a Classification. The return
    // string is owned by ClassificationToString and should not be deleted by the
    // caller.
    char const * ClassificationToString(Classification classification);
}
