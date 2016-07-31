#include <cctype>  // For isspace.
#include <istream>

#include "DocumentFrequencyTable.h"
#include "LoggerInterfaces/Logging.h"

namespace BitFunnel
{
    void SkipWhitespace(std::istream& input)
    {
        while(std::isspace(input.peek()))
        {
            input.get();
        }


    }


    DocumentFrequencyTable::DocumentFrequencyTable(std::istream& input)
    {
        while (input.peek() != EOF)
        {
            char comma;
            Term t(input);
            input >> comma;
            LogAssertB(comma == ',', "Bad input format.");
            double frequency;
            input >> frequency;
            m_entries.push_back(std::make_pair(t, frequency));

            // Need to delete whitespace so that peeking for EOF doesn't get a
            // '\n'.
            SkipWhitespace(input);
        }
    }


    DocumentFrequencyTable::Entry const & DocumentFrequencyTable::operator[](size_t index) const
    {
        return m_entries[index];
    }


    size_t DocumentFrequencyTable::size() const
    {
        return m_entries.size();
    }
}
