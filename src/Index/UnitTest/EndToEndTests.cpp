
#include <fstream>
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/RowId.h"
#include "ChunkEnumerator.h"
#include "Configuration.h"
#include "DocumentDataSchema.h"
#include "Ingestor.h"
#include "IRecycler.h"
#include "ISliceBufferAllocator.h"
#include "Mocks/EmptyTermTable.h"
#include "Recycler.h"
#include "SliceBufferAllocator.h"
// #include "DocumentLengthHistogram.h"


namespace BitFunnel
{
    namespace DocumentLengthHistogramUnitTest
    {
        static std::vector<std::string> const filePaths = {
            "/tmp/chunks/Chunk1",
        };

        void CreateTestFiles(char const * chunkPath, char const * manifestPath)
        {
            {
                std::ofstream chunkStream(chunkPath);

                char const chunk[] =

                    // First document
                    "Title\0Dogs\0\0"
                    "Body\0Dogs\0are\0man's\0best\0friend.\0\0"
                    "\0"

                    // Second document
                    "Title\0Cat\0Facts\0\0"
                    "Body\0The\0internet\0is\0made\0of\0cats.\0\0"
                    "\0"

                    // Third document
                    "Title\0More\0Cat\0Facts\0\0"
                    "Body\0The\0internet\0really\0is\0made\0of\0cats.\0\0"
                    "\0"

                    // End of corpus
                    "\0";

                // Write out sizeof(chunk) - 1 bytes to skip the trailing zero in corpus
                // which is not part of the file format.
                chunkStream.write(chunk, sizeof(chunk) - 1);
                chunkStream.close();
            }

            {
                std::ofstream manifestStream(manifestPath);
                manifestStream << chunkPath << std::endl;
                manifestStream.close();
            }
        }

        //*********************************************************************
        TEST(EndToEndTest, Trivial)
        {
            // TODO: Move this either to a gtest temporary directory test, or
            // modularize out the reading/writing of chunks to streams.
            CreateTestFiles(
                "/tmp/chunks/Chunk1",
                "/tmp/chunks/manifest.txt");

            Configuration config(1);

            std::unique_ptr<IRecycler> recycler =
                std::unique_ptr<IRecycler>(new Recycler());

            // Create dummy SliceBufferAllocator to satisfy interface.
            // TODO: fix constants.
            std::unique_ptr<ISliceBufferAllocator> sliceBufferAllocator =
                std::unique_ptr<ISliceBufferAllocator>(
                    new SliceBufferAllocator(4096, 4096*4));

            static const std::vector<RowIndex>
                rowCounts = { c_systemRowCount, 0, 0, 1, 0, 0, 1 };
            std::shared_ptr<ITermTable const>
                termTable(new EmptyTermTable(rowCounts));

            DocumentDataSchema schema;

            const std::unique_ptr<IIngestor>
                ingestor(Factories::CreateIngestor(schema,
                                                   *recycler,
                                                   *termTable,
                                                   *sliceBufferAllocator));

            ChunkEnumerator chunkEnumerator(filePaths, config, *ingestor, 1);
            chunkEnumerator.WaitForCompletion();

            ingestor->Shutdown();
        }
    }
}
