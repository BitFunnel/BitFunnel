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

#include "BitFunnel/IInterface.h"   // Base class.


namespace BitFunnel
{
    class IConfiguration;
    class IFileManager;
    class IIngestor;
    class IRecycler;
    class ITermTable2;


    //*************************************************************************
    //
    // ISimpleIndex
    //
    // An abstract base class or interface for convenience classes that
    // instantiate and wire up all of the classes needed to form a BitFunnel
    // Index.
    //
    //*************************************************************************
    class ISimpleIndex : public IInterface
    {
    public:
        // Instantiates all of the classes necessary to form a BitFunnel Index.
        // Then starts the index. If forStatistics == true, the index will be
        // started for statistics generation, gathering data for
        //      Document Frequency Table
        //      Cumulative Term Counts
        //      Document Length Histogram
        //      Indexed Idf Table
        // If forStatistics == false, the index will be started for document
        // ingestion and query processing. In this case, it will read files
        // like
        //      Term Table
        //      Indexed Idf Table
        //
        // Note: this method starts a background thread for the IRecycler.
        // This thread is shut down in StopIndex().
        virtual void StartIndex(bool forStatistics) = 0;

        // Performs an orderly shutdown, then tears down all of the classes
        // created by StartIndex(). Must be called before class destruction.
        virtual void StopIndex() = 0;

        virtual IConfiguration const & GetConfiguration() const = 0;
        virtual IFileManager & GetFileManager() const = 0;
        virtual IIngestor & GetIngestor() const = 0;
        virtual IRecycler & GetRecycler() const = 0;
        virtual ITermTable2 const & GetTermTable() const = 0;
    };
}
