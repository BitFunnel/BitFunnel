
#include "DocumentLengthHistogram.h"


namespace BitFunnel
{
    DocumentLengthHistogram::DocumentLengthHistogram()
    {
    }

    void DocumentLengthHistogram::AddDocument(size_t postingCount)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        ++m_hist[postingCount];
    }
    
    size_t DocumentLengthHistogram::GetValue(size_t postingCount)
    {
        // find value given key
        auto p = m_hist.find(postingCount);
        if(p != m_hist.end()) 
        {
            return m_hist[postingCount];
        }
        else
        {
            return 0;
        }
    }

    void DocumentLengthHistogram::Write(std::ostream& output)
    {
        /*static_assert(
            sizeof(unsigned int) == sizeof(size_t),
            "size_t and unsigned int are different sizes"); */
        CsvTsv::OutputColumn<unsigned int> postingCount("Postings", "Number of postings.");
       
        CsvTsv::CsvTableFormatter formatter(output);
        CsvTsv::TableWriter writer(formatter);


        writer.DefineColumn(postingCount);

        writer.WritePrologue();

        for (unsigned i = 0; i < m_hist.size(); ++i)
        {
            postingCount = m_hist[i];
            writer.WriteDataRow();
        }

        writer.WriteEpilogue();
    } 
}    
