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


#pragma once

#include <memory>                   // Uses shared_ptr<T>

#include "BitFunnel/NonCopyable.h"

namespace BitFunnel
{
    typedef unsigned long long SerialNumber;

    //*************************************************************************
    //
    // ITokenListener is an abstract class or interface for classes that 
    // receive notifications when Tokens are destroyed.
    //
    //*************************************************************************
    class ITokenListener
    {
    public:
        
        virtual ~ITokenListener() {};

        // Notification of the SerialNumber of a Token that was just destroyed.
        virtual void OnTokenComplete(SerialNumber serialNumber) = 0;
    };


    //*************************************************************************
    //
    // ITokenTracker is an abstract class or interface for classes that track 
    // the lifetimes of Tokens in a set. A set of Tokens is said to be complete
    // when all Tokens in the set have been destroyed. ITokenTracker provides
    // both polling and blocking methods to assess Token set completion.
    // Classes that implement ITokenTracker usually also implement 
    // ITokenListener to assist in tracking the individual Tokens in the set.
    //
    //*************************************************************************
    class ITokenTracker
    {
    public:

        virtual ~ITokenTracker() {};

        // Returns true if all of the Tokens in the set have been destroyed.
        // This method is thread-safe.
        virtual bool IsComplete() const = 0;

        // Blocks until all of the Tokens in the set have been destroyed.
        // This method is thread-safe.
        virtual void WaitForCompletion() = 0;
    };


    //*************************************************************************
    //
    // ITokenManager is an abstract base class or interface for classes that 
    // distribute tokens and track their lifetimes. Tokens are used to 
    // coordinate a particular activity happening in the system which can
    // potentially be in transition between states. Tokens are not a way to 
    // enforce permissions of their owners on various objects in the system. 
    // Instead, they provide a way to initiate a global change of state
    // and ensure that the change has propagated across the system before
    // performing activity which depends on the completion of this state 
    // change.
    //
    // ITokenManager assists in implementing a limited form of eventual 
    // consistency where background threads restructure shared data into a 
    // newer version while supporting operations from foreground threads that
    // expect either the old version or the new version. 
    //
    // In this model, a foreground thread wishing to perform an operation on 
    // the data is allocated a Token guranteeing availability of a specific 
    // version of the data for the lifetime of the Token. The thread should 
    // hold the Token for the duration of the operation and then release it. 
    //
    // A data version change is initiated by increasing the version number. At 
    // this point all new foreground operations will expect the new version, 
    // but some foreground operations already underway will still require an 
    // older version. Once the version number has been increased, an 
    // ITokenTracker is generated to track the set of Tokens currently in 
    // existence. This set will include Tokens associated with the old version
    // and the new version. At this point, restructuring can proceed as long as
    // it doesn't invalidate data required for old version operations. Once the
    // data has been fully restructured, it is necessary to wait for the 
    // ITokenTracker to complete, indicating that there are no more outstanding
    // old version operations. At this point, the data required for old version 
    // operations is no longer necessary and can be discarded.
    // 
    // ITokenManager supports two main scenarios
    // - Track owners of tokens issued before a specific cutoff time. 
    //   In this case the owner of the token manager wants to roll out
    //   an update to some data structure or change a state in a thread safe
    //   manner. Typically it performs some thread safe pointer exchange, or 
    //   switch to a new state, which is immediately seen by all new workers,
    //   but also acknowledges that there may be in-flight workers which are 
    //   still using the old value of the pointer/state. By tracking the owners
    //   of old Tokens, an algorithm can ensure that all the worker threads 
    //   have switched completely to the new state at which point it is safe to
    //   deprecate the old state or release memory held by previous data 
    //   structure.
    // - Pause distribution of new tokens. This is done when the owner wants
    //   to acquire exclusive permissions on some object. Typically this is
    //   required for a very short time to avoid performance hit. Pausing 
    //   new tokens does not drain old tokens - it is a responsibility of 
    //   the caller to do that.
    //
    // There are 3 categories of the worker threads that we need to track
    // - Ingestion threads
    // - Random access write threads
    // - Query threads
    //
    // All methods are thread-safe.
    //
    // DESIGN NOTE: Before destroying the ITokenManager, users need to call
    // Shutdown(). Trying to RequestToken after a Shutdown() has been called 
    // will LogAbort.
    //
    //*************************************************************************
    class Token;

    class ITokenManager
    {
    public:

        virtual ~ITokenManager() {};

        // Requests a new token from the manager.
        virtual Token RequestToken() = 0;

        // Returns an std::shared_ptr to an ITokenTracker for the set of Tokens
        // in existance at the time of the call. The ITokenManager retains a 
        // std::shared_ptr to the ITokenTracker, and the other copy is given
        // out to a caller of this method. The copy that the manager has, will 
        // be automatically released when the ITokenTracker transitions to its 
        // complete state. The copy that the caller has, will still be valid
        // and can be used to check for completion status of this tracker.
        virtual const std::shared_ptr<ITokenTracker> StartTracker() = 0;

        // Performs shutdown of the TokenManager. This waits for all of the 
        // tokens in existence to be returned. Users of the TokenManager must
        // call Shutdown() before destroying the object.
        virtual void Shutdown() = 0;
    };


    //*************************************************************************
    //
    // A Token guarantees that a particular invariant will hold for the 
    // lifetime of the Token. The actual invariant is defined by the issuer of 
    // the Token, usually an ITokenMananger. Holders of a Token can safely 
    // assume the invariant holds until they allow the token to be destroyed. 
    // Tokens use the RAII pattern similar to std::unique_ptr, and as such, do 
    // not provide a copy constructor and assignment operator. There can only 
    // be a single copy of a particular Token in existence at any given time.
    //
    // Each Token is assigned a SerialNumber when it is constructed. This 
    // SerialNumber will be passed to the Token's ITokenListener when the Token
    // is destroyed. The Token's issuer is responsible for interpretating the 
    // semantics of SerialNumber. Typically an issuer will allocate sequential 
    // increasing numbers, but there is no specific requirement.
    //
    // DESIGN NOTE: In order to ensure that ITokenListener::OnTokenComplete()
    // is invoked exactly once for each Token issued, it is necessary to ensure
    // that a Token can never be copied. For this reason, Token provides a move
    // constructor, but no copy constructor. When a token is moved to another
    // object, the old copy is made "abandoned". This means that it still 
    // exists, but will not notify its issuer when it is destroyed, as its 
    // semantics have now been transferred into the new object. In particular,
    // it allows returning a token by value and makes sure there is no 
    // notification from the temporary object created in this process. 
    //
    //*************************************************************************
    class Token : private NonCopyable
    {
    public:

        // Creates a token with the given issuer. The issuer gets notified 
        // when this token goes out of scope.
        Token(ITokenListener& issuer, SerialNumber serialNumber);

        // Move constructor.
        Token(Token && other);

        ~Token();

        SerialNumber GetSerialNumber() const;

    private:
        // This token's issuer. This is made a pointer instead of a reference 
        // to accommodate the fact that during the move of a token, the 
        // original copy still calls its destructor. We want exactly one call
        // to the issuer for a serial number, so when the object is moved,
        // we will mark it "dead" by resetting m_issuer to nullptr.
        ITokenListener * m_issuer;

        // Token's serial number.
        const SerialNumber m_serialNumber;
    };
}
