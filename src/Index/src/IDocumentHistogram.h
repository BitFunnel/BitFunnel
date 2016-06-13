#pragma once

#include <map>

#include "BitFunnel/Noncopyable.h"


namespace BitFunnel
{
    class DocumentHistogram : public NonCopyable
    {
    public:
        class DocumentHistogramEntry
        {
        public:
            DocumentHistogramEntry();

            void AddDocument(size_t postingCount);

            size_t GetPostingCountSum() const;
            size_t GetDocumentCount() const;

        private:
            size_t m_postingCountSum;
            size_t m_documentCount;
        };

        virtual DocumentHistogramEntry const & operator[](size_t postingCount) const = 0;
        virtual size_t GetMaxPostingCount() const = 0;

        virtual void AddDocument(size_t postingCount) = 0;
    };
}