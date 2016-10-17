
#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Plan/Factories.h"
#include "SimpleResultsProcessor.h"

namespace BitFunnel
{
    std::unique_ptr<IResultsProcessor> Factories::CreateSimpleResultsProcessor()
    {
        return std::unique_ptr<IResultsProcessor>(new SimpleResultsProcessor);
    }


    void SimpleResultsProcessor::AddResult(uint64_t accumulator,
                                  size_t offset)
    {
        m_addResultValues.push_back(std::make_pair(accumulator, offset));
    }


    bool SimpleResultsProcessor::FinishIteration(void const * sliceBuffer)
    {
        for (auto const & result : m_addResultValues)
        {
            uint64_t acc = result.first;
            size_t offset = result.second;

            size_t bitPos = 0;
            while (acc != 0)
            {
                if (acc & 1)
                {
                    DocIndex docIndex = offset * c_bitsPerQuadword + bitPos;
                    DocumentHandle handle =
                        Factories::CreateDocumentHandle(const_cast<void*>(sliceBuffer), docIndex);
                    m_matches.push_back(handle.GetDocId());
                }
                acc >>= 1;
                ++bitPos;
            }
        }
        m_addResultValues.clear();

        // TODO: don't always return false.
        return false;
    }


    bool SimpleResultsProcessor::TerminatedEarly() const
    {
        return false;
    }
}
