#pragma once

#include <map>
#include <sstream>
#include <string>

#include "BitFunnel/IFileManager.h"
#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/NonCopyable.h"

// TODO: what should we do with this old test?

namespace BitFunnel
{
    class TestParameterizedFile0 : public IParameterizedFile0, NonCopyable
    {
    public:
        TestParameterizedFile0(std::string const & name)
            : m_name(name),
              m_buf(nullptr)
        {
        }


        TestParameterizedFile0(std::string const & name, std::string const & value)
            : m_name(name),
              m_buf(new std::stringbuf(value))
        {
        }


        ~TestParameterizedFile0()
        {
            delete m_buf;
        }


        std::string GetName()
        {
            return m_name;
        }
        

        std::istream* OpenForRead()
        {
            LogAssertB(m_buf != nullptr);

            std::istream* stream = new std::istream(m_buf);
            stream->seekg(0, std::istream::beg);
            return stream;
        }


        std::ostream* OpenForWrite()
        {
            if (m_buf == nullptr)
            {
                m_buf = new std::stringbuf();
            }

            std::ostream* stream = new std::ostream(m_buf);
            return stream;
        }


        std::ostream* OpenTempForWrite()
        {
            return OpenForWrite(); 
        }


        void Commit()
        {
            // do nothing
        }


        bool Exists()
        {
            return m_buf != nullptr;
        }


        void Delete()
        {
            delete m_buf;
            m_buf = nullptr; 
        }


    private:
        std::string m_name;
        std::stringbuf* m_buf;
    };


    template <typename P1>
    class TestParameterizedFile1 : public IParameterizedFile1<P1>, NonCopyable
    {
    public:
        TestParameterizedFile1(std::string const & name)
            : m_name(name)
        {
        }


        ~TestParameterizedFile1()
        {
            for (std::map<std::string, std::stringbuf*>::iterator it = m_buffers.begin(); it != m_buffers.end(); ++it)
            {
                delete it->second;
            }
        }


        std::string GetName(P1 p1)
        {
            std::stringstream ss;
            ss << m_name << "-" << p1;
            return ss.str();
        }


        std::istream* OpenForRead(P1 p1)
        {
            std::stringbuf*& buf = m_buffers[GetName(p1)];
            LogAssertB(buf != nullptr)

            std::istream* stream = new std::istream(buf);
            stream->seekg(0, std::istream::beg);

            return stream;
        }


        std::ostream* OpenForWrite(P1 p1)
        {
            std::stringbuf*& buf = m_buffers[GetName(p1)];
            if (buf == nullptr)
            {
                buf = new std::stringbuf();
            }

            std::ostream* stream = new std::ostream(buf);

            return stream;
        }


        std::ostream* OpenTempForWrite(P1 p1)
        {
            return OpenForWrite(p1); 
        }


        void Commit(P1)
        {
            // do nothing
        }


        bool Exists(P1 p1)
        {
            return m_buffers.find(GetName(p1)) != m_buffers.end(); 
        }


        void Delete(P1 p1)
        {
            std::map<std::string, std::stringbuf*>::iterator it = m_buffers.find(GetName(p1));
            
            if (it != m_buffers.end())
            {
                delete it->second;
                m_buffers.erase(it);
            }
        }


    private:
        std::string m_name;
        std::map<std::string, std::stringbuf*> m_buffers;
    };


    template <typename P1, typename P2>
    class TestParameterizedFile2 : public IParameterizedFile2<P1, P2>, NonCopyable
    {
    public:
        TestParameterizedFile2(std::string const & name)
            : m_name(name) 
        {
        }


        ~TestParameterizedFile2()
        {
            for (std::map<std::string, std::stringbuf*>::iterator it = m_buffers.begin(); it != m_buffers.end(); ++it)
            {
                delete it->second;
            }
        }


        std::string GetName(P1 p1, P2 p2)
        {
            std::stringstream ss;
            ss << m_name << "-" << p1 << "-" << p2;
            return ss.str();
        }


        std::istream* OpenForRead(P1 p1, P2 p2)
        {
            std::stringbuf*& buf = m_buffers[GetName(p1, p2)];
            LogAssertB(buf != nullptr);
            
            std::istream* stream = new std::istream(buf);
            stream->seekg(0, std::istream::beg);

            return stream;
        }


        std::ostream* OpenForWrite(P1 p1, P2 p2)
        {
            std::stringbuf*& buf = m_buffers[GetName(p1, p2)];
            if (buf == nullptr)
            {
                buf = new std::stringbuf();
            }

            std::ostream* stream = new std::ostream(buf);

            return stream;
        }


        std::ostream* OpenTempForWrite(P1 p1, P2 p2)
        {
            return OpenForWrite(p1, p2); 
        }
        

        void Commit(P1, P2)
        {
            // do nothing
        }


        bool Exists(P1 p1, P2 p2)
        {
            return m_buffers.find(GetName(p1, p2)) != m_buffers.end(); 
        }


        void Delete(P1 p1, P2 p2)
        {
            std::map<std::string, std::stringbuf*>::iterator it = m_buffers.find(GetName(p1, p2));
            
            if (it != m_buffers.end())
            {
                delete it->second;
                m_buffers.erase(it);
            }
        }

    private:
        std::string m_name;
        std::map<std::string, std::stringbuf*> m_buffers;
    };
}

