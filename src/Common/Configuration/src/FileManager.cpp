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
                             char const * backupDirectory)
        : m_documentHistogram(new ParameterizedFile0(intermediateDirectory, "DocumentHistogram", ".csv" )),
          m_docTable(new ParameterizedFile1<ShardId>(indexDirectory, "DocTable", ".bin" )),
          m_indexSlice(new ParameterizedFile2<ShardId, SliceId>(backupDirectory, "IndexSlice", ".bin"))
    {
    }

        /* TODO: should we return an object even though this is NonCopyable?
    FileDescriptor0 FileManager::DocumentHistogram()
    {
        return FileDescriptor0(*m_documentHistogram);
    }


    FileDescriptor1<ShardId> FileManager::DocTable(ShardId shard)
    {
        return FileDescriptor1<ShardId>(*m_docTable, shard);
    }


    FileDescriptor2<ShardId, SliceId> FileManager::IndexSlice(ShardId shard, SliceId slice)
    {
        return FileDescriptor2<ShardId, SliceId>(*m_indexSlice, shard, slice);
    }
        */
}
