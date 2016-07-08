#include "ConstructorDestructorCounter.h"


namespace BitFunnel
{
    unsigned ConstructorDestructorCounter::m_constructorCount = 0;
    unsigned ConstructorDestructorCounter::m_destructorCount = 0;


    ConstructorDestructorCounter::ConstructorDestructorCounter()
    {
        m_constructorCount++;
    }


    ConstructorDestructorCounter::~ConstructorDestructorCounter()
    {
        m_destructorCount++;
    }


    void ConstructorDestructorCounter::ClearCount()
    {
        m_constructorCount = 0;
        m_destructorCount = 0;
    }


    unsigned ConstructorDestructorCounter::GetConstructorCount()
    {
        return m_constructorCount;
    }


    unsigned ConstructorDestructorCounter::GetDestructorCount()
    {
        return m_destructorCount;
    }
}
