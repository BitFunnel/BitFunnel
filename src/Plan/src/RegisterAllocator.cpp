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

#include <algorithm>            // For std::max().
#include <new>

#include "BitFunnel/Allocators/IAllocator.h"
#include "CompileNode.h"
#include "LoggerInterfaces/Logging.h"
#include "RegisterAllocator.h"

// TODO: get rid of dynamic_cast?

namespace BitFunnel
{
    //*************************************************************************
    //
    // RegisterAllocator
    //
    //*************************************************************************
    RegisterAllocator::RegisterAllocator()
        : m_rowCount(0),
          m_registerCount(0),
          m_registerBase(0),
          m_mapping(nullptr),
          m_abstractRows(nullptr),
          m_registersAllocated(0),
          m_rowIdsByRegister(nullptr)
    {
    }


    RegisterAllocator::RegisterAllocator(CompileNode const & root,
                                         unsigned rowCount,
                                         unsigned registerBase,
                                         unsigned registerCount,
                                         IAllocator& allocator)
        : m_rowCount(rowCount),
          m_registerCount(registerCount),
          m_registerBase(registerBase)
    {
        m_rows = reinterpret_cast<Entry*>(allocator.Allocate(sizeof(Entry)
                                                             * m_rowCount));
        for (unsigned i = 0 ; i < m_rowCount; ++i)
        {
            new (m_rows + i) Entry(i);
        }

        m_abstractRows =
            reinterpret_cast<AbstractRow*>(allocator.Allocate(sizeof(AbstractRow)
                                                              * m_rowCount));

        CollectRows(root, 0, 1);

        std::sort(m_rows, m_rows + m_rowCount);

        // Generate mapping from abstract row id to position in register
        // allocation sort. The first positions in this sort will be allocated
        // to registers.
        m_mapping = reinterpret_cast<unsigned*>(allocator.Allocate(sizeof(unsigned)
                                                                   * m_rowCount));
        for (unsigned i = 0 ; i < m_rowCount; ++i)
        {
            m_mapping[m_rows[i].GetId()] = i;
        }

        // Generate a mapping from register number (starting at 0) to abstract
        // row id.
        m_registersAllocated = 0;
        m_rowIdsByRegister = reinterpret_cast<unsigned*>(allocator.Allocate(sizeof(unsigned)
                                                                            * m_registerCount));
        for (unsigned i = 0; i < m_rowCount && i < m_registerCount; ++i)
        {
            if (m_rows[i].IsUsed())
            {
                m_rowIdsByRegister[m_registersAllocated++] = m_rows[i].GetId();
            }
            else
            {
                break;
            }
        }
    }


    bool RegisterAllocator::IsRegister(unsigned id) const
    {
        return (m_mapping != nullptr) && (m_mapping[id] < m_registerCount);
    }


    unsigned RegisterAllocator::GetRegister(unsigned id) const
    {
        LogAssertB(id < m_rowCount,
                   "id must be < m_rowCount.");
        LogAssertB(m_mapping != nullptr,
                   "m_mapping nullptr.");
        return m_mapping[id] + m_registerBase;
    }


    // TODO: Add unit test for this method.
    unsigned RegisterAllocator::GetRegistersAllocated() const
    {
        return m_registersAllocated;
    }


    // TODO: Add unit test for this method.
    unsigned RegisterAllocator::GetRowIdFromRegister(unsigned reg) const
    {
        LogAssertB(reg < m_registersAllocated,
                   "reg number overflow.");
        LogAssertB(m_rowIdsByRegister != nullptr,
                   "m_rowIdsByRegister nullptr.");
        return m_rowIdsByRegister[reg];
    }


    AbstractRow const & RegisterAllocator::GetRow(unsigned id) const
    {
        LogAssertB(id < m_rowCount,
                   "id overflow.");
        LogAssertB(m_abstractRows != nullptr,
                   "m_abstractRows nullptr.");
        return m_abstractRows[id];
    }


    void RegisterAllocator::CollectRows(CompileNode const & root,
                                        unsigned depth,
                                        unsigned uses)
    {
        switch (root.GetType())
        {
        case CompileNode::opAndRowJz:
            {
                CompileNode::AndRowJz const & node =
                    dynamic_cast<CompileNode::AndRowJz const &>(root);
                unsigned id = node.GetRow().GetId();
                LogAssertB(id < m_rowCount,
                           "id overflow.");
                m_rows[id].UpdateDepth(depth, uses);
                new (m_abstractRows + id) AbstractRow(node.GetRow());
                CollectRows(node.GetChild(), depth + 1, uses);
            }
            break;
        case CompileNode::opLoadRowJz:
            {
                CompileNode::LoadRowJz const & node =
                    dynamic_cast<CompileNode::LoadRowJz const &>(root);
                unsigned id = node.GetRow().GetId();
                LogAssertB(id < m_rowCount,
                           "id overflow.");
                m_rows[id].UpdateDepth(depth, uses);
                new (m_abstractRows + id) AbstractRow(node.GetRow());
                CollectRows(node.GetChild(), depth + 1, uses);
            }
            break;
        case CompileNode::opOr:
            {
                CompileNode::Or const & node =
                    dynamic_cast<CompileNode::Or const &>(root);
                CollectRows(node.GetLeft(), depth, uses);
                CollectRows(node.GetRight(), depth, uses);
            }
            break;
        case CompileNode::opRankDown:
            {
                CompileNode::RankDown const & node =
                    dynamic_cast<CompileNode::RankDown const &>(root);
                CollectRows(node.GetChild(), depth, uses << node.GetDelta());
            }
            break;
        case CompileNode::opReport:
            {
                CompileNode::Report const & node =
                    dynamic_cast<CompileNode::Report const &>(root);
                CompileNode const * child = node.GetChild();
                if (child != nullptr)
                {
                    CollectRows(*child, depth, uses);
                }
            }
            break;
        case CompileNode::opAndTree:
            {
                CompileNode::AndTree const & node =
                    dynamic_cast<CompileNode::AndTree const &>(root);
                CollectRows(node.GetLeft(), depth + 1, uses);
                CollectRows(node.GetRight(), depth + 1, uses);
            }
            break;
        case CompileNode::opLoadRow:
            {
                CompileNode::LoadRow const & node =
                    dynamic_cast<CompileNode::LoadRow const &>(root);
                unsigned id = node.GetRow().GetId();
                LogAssertB(id < m_rowCount,
                           "id overflow.");
                m_rows[id].UpdateDepth(depth, uses);
                new (m_abstractRows + id) AbstractRow(node.GetRow());
            }
            break;
        case CompileNode::opOrTree:
            {
                CompileNode::OrTree const & node =
                    dynamic_cast<CompileNode::OrTree const &>(root);
                CollectRows(node.GetLeft(), depth + 1, uses);
                CollectRows(node.GetRight(), depth + 1, uses);
            }
            break;
        case CompileNode::opNot:
            {
                CompileNode::Not const & node =
                    dynamic_cast<CompileNode::Not const &>(root);
                CollectRows(node.GetChild(), depth, uses);
            }
            break;
        default:
            LogAbortB("Unknown node type.");
            break;
        }
    }


    //*************************************************************************
    //
    // RegisterAllocator::Entry
    //
    //*************************************************************************
    RegisterAllocator::Entry::Entry(unsigned id)
        : m_id(id),
          m_depth(c_noAssociatedRow),
          m_uses(0)
    {
    }


    void RegisterAllocator::Entry::UpdateDepth(unsigned depth, unsigned uses)
    {
        if (depth < m_depth)
        {
            m_depth = depth;
            m_uses = uses;
        }
        else if (depth == m_depth)
        {
            m_uses += uses;
        }
    }


    bool RegisterAllocator::Entry::operator<(Entry const & other) const
    {
        if (m_depth == other.m_depth)
        {
            return m_uses > other.m_uses;
        }
        else
        {
            return m_depth < other.m_depth;
        }
    }


    unsigned RegisterAllocator::Entry::GetDepth() const
    {
        return m_depth;
    }


    unsigned RegisterAllocator::Entry::GetId() const
    {
        return m_id;
    }


    unsigned RegisterAllocator::Entry::GetUses() const
    {
        return m_uses;
    }


    bool RegisterAllocator::Entry::IsUsed() const
    {
        return m_depth != c_noAssociatedRow;
    }
}
