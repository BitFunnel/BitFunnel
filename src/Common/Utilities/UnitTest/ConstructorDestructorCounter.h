#pragma once

namespace BitFunnel
{
    //*************************************************************************
    //
    // ConstructorDestructorCounter is a unit test helper class that counts
    // the number of times its constructor and destructor are called.
    //
    // WARNING: This class is not threadsafe.
    //
    //*************************************************************************
    class ConstructorDestructorCounter
    {
    public:
        ConstructorDestructorCounter();
        ~ConstructorDestructorCounter();

        // Sets the constructor and destructor call counts to zero.
        static void ClearCount();

        // Returns the number of calls to the constructor on all instances
        // of ConstructorDestructorCounter since the last call to ClearCount().
        static unsigned GetConstructorCount();

        // Returns the number of calls to the destructor on all instances
        // of ConstructorDestructorCounter since the last call to ClearCount().
        static unsigned GetDestructorCount();

    private:
        static unsigned m_constructorCount;
        static unsigned m_destructorCount;
    };
}
