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


#include <fstream>

#include "BitFunnel/Configuration/Factories.h"
#include "FileManager.h"
#include "LoggerInterfaces/Logging.h"
#include "ParameterizedFile.h"


namespace BitFunnel
{
    std::unique_ptr<IFileManager>
        Factories::CreateFileManager(char const * intermediateDirectory,
                                     char const * indexDirectory,
                                     char const * backupDirectory)
    {
        return std::unique_ptr<IFileManager>(new FileManager(intermediateDirectory,
                                                             backupDirectory,
                                                             indexDirectory));
    }


    FileManager::FileManager(char const * intermediateDirectory,
                             char const * indexDirectory,
                             char const * /*backupDirectory*/)
        : m_cumulativePostingCounts(new ParameterizedFile1(intermediateDirectory,
                                                           "CumulativePostingCounts",
                                                           ".csv")),
          m_documentLengthHistogram(new ParameterizedFile0(intermediateDirectory,
                                                           "DocumentLengthHistogram",".csv" )),
          m_docFreqTable(new ParameterizedFile1(indexDirectory, "DocFreqTable", ".csv"))
        //m_docTable(new ParameterizedFile1(indexDirectory, "DocTable", ".bin")),
        //m_indexSlice(new ParameterizedFile2(backupDirectory, "IndexSlice", ".bin"))
    {
    }


    FileDescriptor0 FileManager::DocumentLengthHistogram()
    {
        return FileDescriptor0(*m_documentLengthHistogram);
    }


    FileDescriptor1 FileManager::CumulativePostingCounts(size_t shard)
    {
        return FileDescriptor1(*m_cumulativePostingCounts, shard);
    }


    FileDescriptor1 FileManager::DocFreqTable(size_t shard)
    {
        return FileDescriptor1(*m_docFreqTable, shard);
    }

    //FileDescriptor1 FileManager::DocTable(size_t shard)
    //{
    //    return FileDescriptor1(*m_docTable, shard);
    //}


    //FileDescriptor2 FileManager::IndexSlice(size_t shard, size_t slice)
    //{
    //    return FileDescriptor2(*m_indexSlice, shard, slice);
    //}
}
