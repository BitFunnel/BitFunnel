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

#include <sstream>
#include "ChunkReader.h"


namespace BitFunnel
{
    namespace Mocks
    {
        // Parses some chunk data, and counts IEvents events as they happen.
        // For example, if the chunk data contains 5 well-formed streams, this
        // class should have counted 5 OnStreamEnter and OnStreamExit events.
        class ChunkEventTracer : public ChunkReader::IEvents
        {
        public:
            ChunkEventTracer(std::vector<char> const & chunkData)
            {
                ChunkReader(chunkData, *this);
            }


            std::string Trace()
            {
                return m_trace.str();
            }


            //
            // ChunkReader::IEvents methods.
            //
            virtual void OnFileEnter() override
            {
                m_trace << "OnFileEnter" << std::endl;
            }


            void OnDocumentEnter(DocId id) override
            {
                m_trace << "OnDocumentEnter;DocId: '"
                        << id
                        << "'"
                        << std::endl;
            }


            void OnStreamEnter(char const * streamName) override
            {
                m_trace << "OnStreamEnter;streamName: '"
                        << streamName
                        << "'"
                        << std::endl;
            }


            void OnTerm(char const * term) override
            {
                m_trace << "OnTerm;term: '"
                        << term
                        << "'"
                        << std::endl;
            }


            void OnStreamExit() override
            {
                m_trace << "OnStreamExit" << std::endl;
            }


            void OnDocumentExit() override
            {
                m_trace << "OnDocumentExit" << std::endl;
            }


            void OnFileExit() override
            {
                m_trace << "OnFileExit" << std::endl;
            }


        private:
            std::stringstream m_trace;
        };
    }
}
