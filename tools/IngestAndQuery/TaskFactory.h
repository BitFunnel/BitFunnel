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
    class IThreadBase;

    class ITask : public IInterface
    {
    public:
        typedef size_t Id;
        enum Type {
            Synchronous,
            Asynchronous,
            Exit
        };

        typedef
            std::unique_ptr<ITask>(*Create)(
                Environment & environment,
                Id id,
                std::vector<std::string> const & tokens);

        class Descriptor
        {
        public:
            Descriptor(char const * name,
                       char const * oneLineDescription,
                       char const * verboseDescription,
                       ITask::Create creator)
                : m_name(name),
                m_oneLineDescription(oneLineDescription),
                m_verboseDescription(verboseDescription),
                m_creator(creator)
            {
            }

            char const * GetName() const
            {
                return m_name;
            }

            char const * GetOneLineDescription() const
            {
                return m_oneLineDescription;
            }

            char const * GetVerboseDescription() const
            {
                return m_verboseDescription;
            }

            std::unique_ptr<ITask> Create(
                Environment & environment,
                ITask::Id id,
                std::vector<std::string> const & tokens) const
            {
                return m_creator(environment, id, tokens);
            }

        private:
            char const * m_name;
            char const * m_oneLineDescription;
            char const * m_verboseDescription;
            ITask::Create m_creator;
        };


        virtual Type GetType() const = 0;

        virtual Id GetId() const = 0;

        virtual Environment & GetEnvironment() const = 0;

        virtual void Execute() = 0;
    };


    class ITaskFactory : public IInterface
    {
    public:
        virtual void Register(std::unique_ptr<ITask::Descriptor> descriptor) = 0;

        virtual std::unique_ptr<ITask> CreateTask(char const * line) = 0;

        virtual void Help(std::ostream& output, char const * command) const = 0;

    };


    class TaskFactory : public ITaskFactory, public NonCopyable
    {
    public:
        TaskFactory(Environment & environment);

        virtual void Register(std::unique_ptr<ITask::Descriptor> descriptor) override;

        virtual std::unique_ptr<ITask> CreateTask(char const * line) override;

        virtual void Help(std::ostream& output, char const * command) const override;

        ITask::Id GetNextTaskId() const;

    private:
        static std::vector<std::string> Tokenize(char const * text);

        Environment & m_environment;
        ITask::Id m_nextId;
        size_t m_maxNameLength;
        std::map<std::string, std::unique_ptr<ITask::Descriptor>> m_taskMap;
    };
}
