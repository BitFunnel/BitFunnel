#include <ctype.h>      // For isspace().

#include "SameExceptForWhitespace.h"


namespace BitFunnel
{
    static void SkipWhite(char const * & s)
    {
        while (isspace(*s))
        {
            s++;
        }
    }


    bool SameExceptForWhitespace(const char* a, const char* b)
    {
        while (*a != 0 && *b != 0)
        {
            SkipWhite(a);
            SkipWhite(b);

            if (*a == 0 || *b == 0)
            {
                break;
            }

            if (*a != *b)
            {
                return false;
            }

            ++a;
            ++b;
        }

        SkipWhite(a);
        SkipWhite(b);
        return (*a == 0 && *b == 0);
    }
}
