#pragma once


namespace BitFunnel
{
    class AbstractRow;
    class CompileNode;
    class IAllocator;

    //*************************************************************************
    //
    // RegisterAllocator assigns regsiters to rows in a tree of CompileNodes.
    // Rows closer to the root are assigned registers before rows deeper in
    // the tree. For rows that are at the same depth, the row that is used more
    // often is given a higher priority for register allocation. Typically the
    // row usage is most impacted by the RankDown operation which will execute
    // a subtree 2^delta times. Multiple uses can also come from multiple
    // references in the tree itself. This can happen when multiplying out
    // an And or Ors, e.g. (a + b)(c + d) results in a(c + d) + b(c + d) which
    // uses c and d twice.
    //
    //*************************************************************************
    class RegisterAllocator
    {
    public:
        // Constructs a register allocator that allocates no registers.
        RegisterAllocator();

        // Constructs a register allocator, based on a CompileNode tree.
        // The rowCount parameter must be at least as large as the number of
        // distinct rows in the tree. Up to registerCount registers will be
        // allocated, with register numbers starting at registerBase.
        RegisterAllocator(CompileNode const & root,
                          unsigned rowCount,
                          unsigned registerBase,
                          unsigned registerCount,
                          IAllocator& allocator);

        // Returns true if the abstract row with the specified id has been
        // assigned a register.
        bool IsRegister(unsigned id) const;

        // Retunrs the register number of the abstract row with the specified
        // id.
        unsigned GetRegister(unsigned id) const;

        // Returns the number of registers actually allocated.
        unsigned GetRegistersAllocated() const;

        // Returns the abstract row id for the specified register (where the
        // register number starts at zero). Note that reg must be less than
        // the number of registers allocated.
        unsigned GetRowIdFromRegister(unsigned reg) const;

        // Returns the abstract row associated with a particular id.
        AbstractRow const & GetRow(unsigned id) const;

    private:
        void CollectRows(CompileNode const & node,
                         unsigned depth,
                         unsigned uses);

        class Entry
        {
        public:
            Entry(unsigned id);

            void UpdateDepth(unsigned depth, unsigned uses);

            bool operator<(Entry const & other) const;

            unsigned GetDepth() const;
            unsigned GetId() const;
            unsigned GetUses() const;

            bool IsUsed() const;

        private:
            // This row's identifier.
            unsigned m_id;

            // The number of rows that are evaluated before this row.
            unsigned m_depth;

            // Number of times the row is used at m_depth from the root.
            unsigned m_uses;

            // m_depth is set to c_noAssociatedRow to indicate that this entry
            // is not associated with any row. c_noAssociatedRow is defined as
            // the largest unsigned value in order to push unassociated rows to
            // the end of the register allocation sort.
            static const unsigned c_noAssociatedRow = ~0U;
        };

        // Total number of rows in the plan. Will be used to size m_mapping.
        unsigned m_rowCount;

        // Total number of registers available.
        unsigned m_registerCount;

        // First available register number. Algorithm will allocate to a
        // contiguous block of m_registerCount registers, starting at
        // m_registerBase.
        unsigned m_registerBase;

        Entry * m_rows;
        unsigned * m_mapping;

        AbstractRow * m_abstractRows;

        unsigned m_registersAllocated;
        unsigned * m_rowIdsByRegister;
    };
}
