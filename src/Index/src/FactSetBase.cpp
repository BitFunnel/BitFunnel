#include <cstring>
#include <memory>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Row.h"                  // For c_systemRowCount.
#include "FactSetBase.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // IFactSet::FactInfo.
    //
    //*************************************************************************
    FactSetBase::FactInfo::FactInfo(char const * friendlyName,
                                    bool isMutable,
                                    FactHandle handle)
        : m_friendlyName(friendlyName),
          m_handle(handle),
          m_isMutable(isMutable)
    {
    }


    FactSetBase::FactInfo::FactInfo(FactInfo const & other)
        : m_friendlyName(other.m_friendlyName),
          m_handle(other.m_handle),
          m_isMutable(other.m_isMutable)
    {
    }


    char const * FactSetBase::FactInfo::GetFriendlyName() const
    {
        return m_friendlyName.c_str();
    }


    FactHandle FactSetBase::FactInfo::GetHandle() const
    {
        return m_handle;
    }


    bool FactSetBase::FactInfo::IsMutable() const
    {
        return m_isMutable;
    }


    //*************************************************************************
    //
    // FactSetBase.
    //
    //*************************************************************************
    std::unique_ptr<IFactSet> Factories::CreateFactSet()
    {
        return std::unique_ptr<IFactSet>(new FactSetBase());
    }


    FactSetBase::FactSetBase()
    {
    }


    FactHandle FactSetBase::DefineFact(char const * friendlyName,
                                       bool isMutable)
    {
        for (size_t i = 0; i < m_facts.size(); ++i)
        {
            if (0 == strcmp(m_facts[i].GetFriendlyName(), friendlyName))
            {
                throw RecoverableError("Duplicate fact name definition.");
            }
        }

        // Fact handles are allocated starting with the value of c_systemRowCount
        // to accommodate system internal rows. System internal rows are not visible to
        // the user and are not stored in IFactSet explicitly.
        FactHandle handle = static_cast<FactHandle>(m_facts.size()) + c_systemRowCount;
        m_facts.emplace_back(friendlyName, isMutable, handle);

        return handle;
    }


    unsigned FactSetBase::GetCount() const
    {
        return static_cast<unsigned>(m_facts.size());
    }


    FactHandle FactSetBase::operator[](unsigned index) const
    {
        return m_facts.at(index).GetHandle();
    }


    bool FactSetBase::IsMutable(FactHandle fact) const
    {
        FactInfo const & factInfo = GetFactInfoByHandle(fact);
        return factInfo.IsMutable();
    }


    char const * FactSetBase::GetFriendlyName(FactHandle fact) const
    {
        FactInfo const & factInfo = GetFactInfoByHandle(fact);
        return factInfo.GetFriendlyName();
    }


    FactSetBase::FactInfo const & FactSetBase::GetFactInfoByHandle(FactHandle handle) const
    {
        // Facts are assigned sequential handles starting at c_systemRowCount.
        // To avoid O(n) complexity in looking up a fact in the list, obtain an index of a
        // fact from its handle.
        const unsigned factIndex = static_cast<unsigned>(handle) - c_systemRowCount;
        return m_facts.at(factIndex);
    }
}
