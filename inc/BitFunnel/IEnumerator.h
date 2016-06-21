#pragma once

namespace BitFunnel
{
    // Polymorphic base interface for IEnumerator<T>.
    class IEnumeratorBase
    {
    public:
        virtual ~IEnumeratorBase() {};

        // Advances the enumerator to the next item in the collection. Note
        // that MoveNext() must be called at the beginning of the enumeration
        // before any calls to the Current() method. MoveNext() returns true
        // if it successfully moved to an item in the collection. MoveNext()
        // returns false when the collection is empty or the last item has
        // been passed.
        virtual bool MoveNext() = 0;

        // Moves the enumerator to the beginning at a position before the first
        // item in the collection.
        virtual void Reset() = 0;
    };

    // IEnumerator is an abstract base class for enumerating collections of
    // items of type T.
    template <typename T>
    class IEnumerator : public virtual IEnumeratorBase
    {
    public:
        virtual ~IEnumerator() {};

        // Returns the item at the current position in the enumerator. Only
        // legal if the most recent call to MoveNext() returned true.
        //
        // DESIGN NOTE: Returns T, rather than const T& to allow enumerator
        // enumerator to synthesize T values (e.g. pairs in a hash table).
        // Collections of heavy-weight items should supply enumerators of
        // pointers to avoid the large copy construction costs.
        virtual T Current() const = 0;
    };
}
