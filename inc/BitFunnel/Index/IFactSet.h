#pragma once

namespace BitFunnel
{
    //*************************************************************************
    //
    // IFactSet is an abstract base class or an interface for classes that 
    // maintain a collection of FactInfo objects that describe document facts.
    // A document fact represents a predicate or assertion that is either true 
    // or false for a given document, e.g. document is in top 20% static rank, 
    // or document was indexed before 5pm.
    //
    // Document facts are typically implemented as private, rank 0 rows in the 
    // index. Document facts may be mutable or immutable. An immutable fact for
    // a particular document is considered constant for the life of the 
    // document. A mutable fact may be changed at any time.
    //
    // The IFactSet is used to configure the TermTable to be able to set and
    // clear (clear - only for mutable facts) facts about documents in the 
    // index and be able to alter queries to include the facts in the query 
    // plan.
    //
    //*************************************************************************
    typedef unsigned __int64 FactHandle;

    class IFactSet
    {
    public:
        virtual ~IFactSet() {};

        // Allocates a fact with sequentially numbered FactHandles starting 
        // at 1.
        virtual FactHandle DefineFact(char const * friendlyName, 
                                      bool isMutable) = 0;

        // Returns the number of facts defined in the IFactSet.
        virtual unsigned GetCount() const = 0;

        // Returns a handle for a fact with the specified index.
        virtual FactHandle operator[](unsigned index) const = 0;

        // Returns whether a fact with the given handle is mutable or not.
        // If a fact with this handle is not found, this function throws.
        virtual bool IsMutable(FactHandle fact) const = 0;

        // Returns a friendly name for a fact with the given handle.
        // If a fact with this handle is not found, this function throws.
        virtual char const * GetFriendlyName(FactHandle fact) const = 0;
    };
}