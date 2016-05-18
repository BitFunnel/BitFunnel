#pragma once

namespace BitFunnel
{
    //*************************************************************************
    //
    // Class ITaskProcessor
    //
    //*************************************************************************
    class ITaskProcessor
    {
    public:
        virtual ~ITaskProcessor() {}

        virtual void ProcessTask(size_t taskId) = 0;
        virtual void Finished() = 0;
    };
}
