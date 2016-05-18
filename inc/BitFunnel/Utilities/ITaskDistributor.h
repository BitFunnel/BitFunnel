#pragma once


namespace BitFunnel
{
    //*************************************************************************
    //
    // ITaskDistributor is an abstract base class or interface for objects that
    // assign tasks to threads.
    //
    // Uses multiple threads to dispatch numbered tasks to a vector of objects
    // that derive from ITaskProcessor.
    //
    // One thread is started for each ITaskProcessor. The ITaskProcessors are
    // assigned task ids via ITaskProcessor::ProcessTask(). Task ids are handed
    // out starting from 0 and increasing until all ITaskProcessors are busy.
    // Each time a thread returns from ProcessTask(), a new task id will be
    // assigned until all tasks have been processed.
    //
    // Task coordinator only knows about task ids. The interpretation of the
    // work associated with a particular task id is up to the ITaskProcessor.
    //
    //*************************************************************************
    class ITaskDistributor
    {
    public:
        virtual ~ITaskDistributor() {};

        // TaskDistributorThreads call TryAllocateTask() to get their next task
        // assignment. If there is work remaining, taskId will be set to the id
        // of the assigned task and the method will return true. If there are
        // no tasks remaining, the method will return false.
        virtual bool TryAllocateTask(size_t& taskId) = 0;

        // Waits a specified amount of time for tasks to complete. Returns true if all threads
        // exited successfully before the timeout period expired.
        virtual bool WaitForCompletion(int timeoutInMs) = 0;
    };
}
