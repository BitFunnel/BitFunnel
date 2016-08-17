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

#include <iosfwd>                   // std::ostream parameter.
#include <map>                      // std::map embedded.
#include <memory>                   // std::unique_ptr return value.
#include <string>
#include <vector>

#include "BitFunnel/IInterface.h"   // Base class.
#include "BitFunnel/Noncopyable.h"  // Base class.


namespace BitFunnel
{
    class Environment;


    class ITask : public IInterface
    {
    public:
        typedef size_t Id;

        enum Type {
            Synchronous,
            Asynchronous,
            Exit
        };


        virtual Type GetType() const = 0;
        virtual Id GetId() const = 0;
        virtual Environment & GetEnvironment() const = 0;
        virtual void Execute() = 0;


        class Documentation
        {
        public:
            Documentation(char const * name,
                          char const * parameters,
                          char const * description)
                : m_name(name),
                m_parameters(parameters),
                m_description(description)
            {
            }

            char const * GetName() const
            {
                return m_name;
            }

            char const * GetParameters() const
            {
                return m_parameters;
            }

            char const * GetDescription() const
            {
                return m_description;
            }

        private:
            char const * m_name;
            char const * m_parameters;
            char const * m_description;
        };
    };


    class TaskFactory : public NonCopyable
    {
    public:
        TaskFactory(Environment & environment);

        virtual std::unique_ptr<ITask> CreateTask(char const * line);
        virtual void Help(std::ostream& output, char const * command) const;
        ITask::Id GetNextTaskId() const;

    private:
        class Descriptor;

    public:
        template <class T>
        void RegisterCommand()
        {
            auto docs = T::GetDocumentation();
            std::unique_ptr<Descriptor> descriptor(
                new Descriptor(T::GetDocumentation(),
                               Create<T>));
            RegisterHelper(std::move(descriptor));
        }

    private:
        void RegisterHelper(std::unique_ptr<Descriptor> descriptor);
        static std::vector<std::string> Tokenize(char const * text);

        Environment & m_environment;
        ITask::Id m_nextId;
        size_t m_maxNameLength;
        std::map<std::string, std::unique_ptr<Descriptor>> m_taskMap;


        typedef std::unique_ptr<ITask>(*Creator)(
            Environment & environment,
            ITask::Id id,
            std::vector<std::string> const & tokens);


        class Descriptor
        {
        public:
            Descriptor(ITask::Documentation documentation,
                       Creator creator)
                : m_documentation(documentation),
                m_creator(creator)
            {
            }

            ITask::Documentation GetDocumentation() const
            {
                return m_documentation;
            }


            std::unique_ptr<ITask> Create(
                Environment & environment,
                ITask::Id id,
                std::vector<std::string> const & tokens)
            {
                return m_creator(environment,
                                 id,
                                 tokens);
            }

        private:
            ITask::Documentation m_documentation;
            Creator m_creator;
        };


        template <class T>
        static std::unique_ptr<ITask> Create(
            Environment & environment,
            ITask::Id id,
            std::vector<std::string> const & tokens)
        {
            return std::unique_ptr<ITask>(new T(environment, id, tokens));
        }
    };
}
