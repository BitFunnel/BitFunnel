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

#include <ostream>                          // std::ostream methods inlined.

#include "BitFunnel/Utilities/Stopwatch.h"  // Stopwatch embedded.


namespace CsvTsv
{
    class CsvTableFormatter;
}


namespace BitFunnel
{
    class QueryInstrumentation
    {
    public:
        class Data;

        inline void QuerySucceeded()
        {
            m_data.m_succeeded = true;
        }

        inline void SetMatchCount(size_t matchCount)
        {
            m_data.m_matchCount = matchCount;
        }

        inline void SetRowCount(size_t rowCount)
        {
            m_data.m_rowCount = rowCount;
        }

        inline void IncrementQuadwordCount()
        {
            ++m_data.m_quadwordCount;
        }

        inline void IncrementQuadwordCount(size_t amount)
        {
            m_data.m_quadwordCount += amount;
        }

        inline void IncrementCacheLineCount(size_t amount)
        {
            m_data.m_cacheLineCount += amount;
        }

        inline void FinishParsing()
        {
            m_data.m_parsingTime = m_stopwatch.ElapsedTime();
        }

        inline void FinishPlanning()
        {
            m_data.m_planningTime = m_stopwatch.ElapsedTime() - m_data.m_parsingTime;
        }

        inline void FinishMatching()
        {
            m_data.m_matchingTime = m_stopwatch.ElapsedTime() - m_data.m_planningTime;
        }

        inline Data & GetData()
        {
            return m_data;
        }

        class Data
        {
        public:
            inline Data()
              : m_succeeded(false),
                m_rowCount(0ull),
                m_matchCount(0ull),
                m_quadwordCount(0ull),
                m_cacheLineCount(0ll),
                m_parsingTime(0.0),
                m_planningTime(0.0),
                m_matchingTime(0.0)
            {
            }

            Data & operator=(Data const & other)
            {
                m_succeeded = other.m_succeeded;
                m_rowCount = other.m_rowCount;
                m_matchCount = other.m_matchCount;
                m_quadwordCount = other.m_quadwordCount;
                m_cacheLineCount = other.m_cacheLineCount;
                m_parsingTime = other.m_parsingTime;
                m_planningTime = other.m_planningTime;
                m_matchingTime = other.m_matchingTime;
                return *this;
            }

            inline size_t GetSucceeded()
            {
                return m_succeeded;
            }

            inline size_t GetRowCount()
            {
                return m_rowCount;
            }

            inline size_t GetMatchCount()
            {
                return m_matchCount;
            }

            inline size_t GetQuadwordCount()
            {
                return m_quadwordCount;
            }

            inline size_t GetCacheLineCount()
            {
                return m_cacheLineCount;
            }

            inline double GetParsingTime()
            {
                return m_parsingTime;
            }

            inline double GetPlanningTime()
            {
                return m_planningTime;
            }

            inline double GetMatchingTime()
            {
                return m_matchingTime;
            }

            static void FormatHeader(CsvTsv::CsvTableFormatter & formatter);
            void Format(CsvTsv::CsvTableFormatter & formatter) const;

        private:
            friend class QueryInstrumentation;

            bool m_succeeded;
            size_t m_rowCount;
            size_t m_matchCount;
            size_t m_quadwordCount;
            size_t m_cacheLineCount;
            double m_parsingTime;
            double m_planningTime;
            double m_matchingTime;
        };

    private:
        Stopwatch m_stopwatch;
        Data m_data;
    };
}
