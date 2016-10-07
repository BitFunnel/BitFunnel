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

#include <cstring>
#include <memory>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/ITermTable.h"
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

        // System terms occupy FactHandles 0..ITermTable::SystemTerm::Last.
        // Therefore, allocate new FactHandles starting at
        // ITermTable::SystemTerm::Count.
        FactHandle handle =
            static_cast<FactHandle>(m_facts.size()
                                    + ITermTable::SystemTerm::Count);
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
        const size_t factIndex = handle;
        return m_facts.at(factIndex);
    }
}
