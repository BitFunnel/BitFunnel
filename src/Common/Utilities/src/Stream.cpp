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
