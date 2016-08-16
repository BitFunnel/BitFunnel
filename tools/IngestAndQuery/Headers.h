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

#include <map>      // std::map embedded.
#include <memory>   // std::unique_ptr return value.
#include <string>   // std::string parameterizes template.
#include <vector>   // std::vector embedded.

#include "BitFunnel/IInterface.h"
#include "BitFunnel/Noncopyable.h"
#include "BitFunnel/Utilities/BlockingQueue.h"
#include "BitFunnel/Utilities/IThreadManager.h"


namespace BitFunnel
{
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
                Id id,
                std::vector<std::string> const & tokens);

        virtual Type GetType() const = 0;

        virtual Id GetId() const = 0;

        virtual void Execute() = 0;
    };


    class ITaskFactory : public IInterface
    {
    public:
        virtual void Register(char const * name,
                              char const * documentation,
                              ITask::Create create) = 0;

        virtual std::unique_ptr<ITask> CreateTask(char const * line) = 0;
    };


    class TaskFactory : public ITaskFactory, public NonCopyable
    {
    public:
        TaskFactory();

        virtual void Register(char const * name,
                              char const * documentation,
                              ITask::Create create) override;

        virtual std::unique_ptr<ITask> CreateTask(char const * line) override;

        ITask::Id GetNextTaskId() const;

    private:
        std::vector<std::string> Tokenize(char const * text);

        ITask::Id m_nextId;
        std::map<std::string, ITask::Create> m_taskMap;
    };


    class TaskPool
    {
    public:
        TaskPool(size_t threadCount);

        bool TryEnqueue(std::unique_ptr<ITask> task);

    private:
        class Thread : public IThreadBase
        {
        public:
            Thread(TaskPool& pool);

            void EntryPoint();

        private:
            TaskPool& m_pool;
        };

        std::vector<IThreadBase*> m_threads;
        std::unique_ptr<IThreadManager> m_threadManager;

        BlockingQueue<std::unique_ptr<ITask>> m_queue;
    };


    class TaskBase : public ITask
    {
    public:
        TaskBase(Id id, Type type);

        virtual Type GetType() const override;

        virtual Id GetId() const override;

    private:
        Id m_id;
        Type m_type;
    };


    class Exit : public TaskBase
    {
    public:
        Exit();

        static void Register(TaskFactory & factory);

        static std::unique_ptr<ITask>
            Create(Id id, std::vector<std::string> const & tokens);


        //
        // ITask methods
        //

        virtual void Execute() override;
    };


    class DelayedPrint : public TaskBase
    {
    public:
        DelayedPrint(Id id, char const * message);

        static void Register(TaskFactory & factory);

        static std::unique_ptr<ITask>
            Create(Id id, std::vector<std::string> const & tokens);


        //
        // ITask methods
        //

        virtual void Execute() override;

    private:
        size_t m_sleepTime;
        std::string m_message;
    };
}
