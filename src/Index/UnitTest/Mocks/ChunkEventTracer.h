
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


            void OnDocumentEnter(DocId id)
            {
                m_trace << "OnDocumentEnter;DocId: '"
                        << id
                        << "'"
                        << std::endl;
            }


            void OnStreamEnter(char const * streamName)
            {
                m_trace << "OnStreamEnter;streamName: '"
                        << streamName
                        << "'"
                        << std::endl;
            }


            void OnTerm(char const * term)
            {
                m_trace << "OnTerm;term: '"
                        << term
                        << "'"
                        << std::endl;
            }


            void OnStreamExit()
            {
                m_trace << "OnStreamExit" << std::endl;
            }


            void OnDocumentExit()
            {
                m_trace << "OnDocumentExit" << std::endl;
            }


            void OnFileExit()
            {
                m_trace << "OnFileExit" << std::endl;
            }


        private:
            std::stringstream m_trace;
        };
    }
}
