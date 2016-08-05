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

#include <istream>
#include <memory>       // std::unique_ptr return value.
#include <ostream>
#include <stddef.h>     // size_t parameter.
#include <string>       // std::string return value.


namespace BitFunnel
{
    class IParameterizedFile0;
    class IParameterizedFile1;
    class IParameterizedFile2;

    class FileDescriptor0;
    class FileDescriptor1;
    class FileDescriptor2;

    //*************************************************************************
    //
    // IFileManager
    //
    // Interface for objects that generate parameterized file names and open
    // and close streams associated with these files.
    //
    // DESIGN NOTE: None of the methods are const in order to allow greater
    // flexibility for mocks and other classes that implement IFileManager.
    //
    // DESIGN NOTE: The methods don't use the GetFoo() naming convention for
    // two reasons. The first is that they are not really getters for
    // properties. The implementation could choose to synthesize new
    // FileDescriptors for each call. The second reason is for brevity.
    //
    //*************************************************************************
    class IFileManager
    {
    public:
        virtual ~IFileManager() {};

        // These methods return descriptors for files that are only
        // parameterized by the IFileManager. The returned FileDescriptor0
        // objects provide methods to generate the file names and open the
        // files.

        //virtual FileDescriptor0 BandTable() = 0;
        //virtual FileDescriptor0 CommonNegatedTerms() = 0;
        //virtual FileDescriptor0 CommonPhrases() = 0;
        //virtual FileDescriptor0 DocFreqTable() = 0;
        virtual FileDescriptor0 DocumentLengthHistogram() = 0;
        //virtual FileDescriptor0 L1RankerConfig() = 0;
        //virtual FileDescriptor0 Manifest() = 0;
        //virtual FileDescriptor0 Model() = 0;
        //virtual FileDescriptor0 PlanDescriptors() = 0;
        //virtual FileDescriptor0 PostingCounts() = 0;
        //virtual FileDescriptor0 ShardDefinition() = 0;
        //virtual FileDescriptor0 ShardDocCounts() = 0;
        //virtual FileDescriptor0 ShardedDocFreqTable() = 0;
        //virtual FileDescriptor0 SortRankerConfig() = 0;
        //virtual FileDescriptor0 StreamNameToSuffixMap() = 0;
        //virtual FileDescriptor0 SuffixToClassificationMap() = 0;
        //virtual FileDescriptor0 ClickStreamSuffixToMarketMap() = 0;
        //virtual FileDescriptor0 TierDefinition() = 0;
        //virtual FileDescriptor0 TermDisposeDefinition() = 0;
        //virtual FileDescriptor0 MetaWordTierHintMap() = 0;
        //virtual FileDescriptor0 TermTableStats() = 0;
        //virtual FileDescriptor0 PostingAndBitStats() = 0;
        //virtual FileDescriptor0 StrengtheningMetawords() = 0;

        // These methods return descriptors for files that are parameterized
        // by a shard number.  The returned FileDescriptor1 objects provide
        // methods to generate the file names and open the files.
        virtual FileDescriptor1 CumulativePostingCounts(size_t shard) = 0;
        virtual FileDescriptor1 DocFreqTable(size_t shard) = 0;
        //virtual FileDescriptor1 DocTable(size_t shard) = 0;
        //virtual FileDescriptor1 ScoreTable(size_t shard) = 0;
        //virtual FileDescriptor1 TermTable(size_t shard) = 0;

        //virtual FileDescriptor2 IndexSlice(size_t shard,
        //                                   size_t slice) = 0;
    };


    class IParameterizedFile0
    {
    public:
        virtual ~IParameterizedFile0() {};

        virtual std::string GetName() = 0;
        virtual std::unique_ptr<std::istream> OpenForRead() = 0;
        virtual std::unique_ptr<std::ostream> OpenForWrite() = 0;
        // virtual std::unique_ptr<std::ostream> OpenTempForWrite() = 0;
        // virtual void Commit() = 0;
        // virtual bool Exists() = 0;
        // virtual void Delete() = 0;
    };


    class IParameterizedFile1
    {
    public:
        virtual ~IParameterizedFile1() {};

        virtual std::string GetName(size_t p1) = 0;
        virtual std::unique_ptr<std::istream> OpenForRead(size_t p1) = 0;
        virtual std::unique_ptr<std::ostream> OpenForWrite(size_t p1) = 0;
        // virtual std::unique_ptr<std::ostream> OpenTempForWrite(size_t p1) = 0;
        // virtual void Commit(size_t p1) = 0;
        // virtual bool Exists(size_t p1) = 0;
        // virtual void Delete(size_t p1) = 0;
    };


    class IParameterizedFile2
    {
    public:
        virtual ~IParameterizedFile2() {};

        virtual std::string GetName(size_t p1, size_t p2) = 0;
        virtual std::unique_ptr<std::istream> OpenForRead(size_t p1, size_t p2) = 0;
        virtual std::unique_ptr<std::ostream> OpenForWrite(size_t p1, size_t p2) = 0;
        // virtual std::unique_ptr<std::ostream> OpenTempForWrite(size_t p1, size_t p2) = 0;
        // virtual void Commit(size_t p1, size_t p2) = 0;
        // virtual bool Exists(size_t p1, size_t p2) = 0;
        // virtual void Delete(size_t p1, size_t p2) = 0;
    };


    // DESIGN NOTE: Deliberately using inline template method definition for
    // brevity.
    class FileDescriptor0
    {
    public:
        FileDescriptor0(IParameterizedFile0& file)
            : m_file(file)
        {
        }

        std::string GetName() { return m_file.GetName(); }
        std::unique_ptr<std::istream> OpenForRead() { return m_file.OpenForRead(); }
        std::unique_ptr<std::ostream> OpenForWrite() { return m_file.OpenForWrite(); }
        // std::unique_ptr<std::ostream> OpenTempForWrite() { return m_file.OpenTempForWrite(); }
        // void Commit() { return m_file.Commit(); }
        // bool Exists() { return m_file.Exists(); }
        // void Delete() { m_file.Delete(); }

    private:
        IParameterizedFile0& m_file;
    };


    // DESIGN NOTE: Deliberately using inline template method definition for
    // brevity.
    class FileDescriptor1
    {
    public:
        FileDescriptor1(IParameterizedFile1& file, size_t p1)
            : m_file(file),
              m_p1(p1)
        {
        }

        std::string GetName() { return m_file.GetName(m_p1); }
        std::unique_ptr<std::istream> OpenForRead() { return m_file.OpenForRead(m_p1); }
        std::unique_ptr<std::ostream> OpenForWrite() { return m_file.OpenForWrite(m_p1); }
        // std::unique_ptr<std::ostream> OpenTempForWrite() { return m_file.OpenTempForWrite(m_p1); }
        // void Commit() { return m_file.Commit(m_p1); }
        // bool Exists() { return m_file.Exists(m_p1); }
        // void Delete() { m_file.Delete(m_p1); }

    private:
        IParameterizedFile1& m_file;
        size_t m_p1;
    };


    // DESIGN NOTE: Deliberately using inline template method definition for
    // brevity.
    class FileDescriptor2
    {
    public:
        FileDescriptor2(IParameterizedFile2& file, size_t p1, size_t p2)
            : m_file(file),
              m_p1(p1),
              m_p2(p2)
        {
        }

        std::string GetName() { return m_file.GetName(m_p1, m_p2); }
        std::unique_ptr<std::istream> OpenForRead() { return m_file.OpenForRead(m_p1, m_p2); }
        std::unique_ptr<std::ostream> OpenForWrite() { return m_file.OpenForWrite(m_p1, m_p2); }
        // std::unique_ptr<std::ostream> OpenTempForWrite() { return m_file.OpenTempForWrite(m_p1, m_p2); }
        // void Commit() { return m_file.Commit(m_p1, m_p2); }
        // bool Exists() { return m_file.Exists(m_p1, m_p2); }
        // void Delete() { m_file.Delete(m_p1, m_p2); }

    private:
        IParameterizedFile2& m_file;
        size_t m_p1;
        size_t m_p2;
    };
}
