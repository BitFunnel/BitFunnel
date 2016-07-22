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

#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <stdio.h>


#include "CsvTsv/Csv.h"
#include "CsvTsv/Tsv.h"
#include "gtest/gtest.h"


namespace CsvTsv
{
    // TODO: Test all forms of valid field values (e.g. 00, -0, +0, etc.)
    // TODO: Test all forms of whitespace.
    // TODO: Tests of error handling for poorly formatted fields.
    // TODO: Test of check for duplicate column name.
    // TODO: Test for valid column names.
    // TODO: Decide whether to support embedded newlines in CSV and TSV.
    // TODO: Add test for rejecting negative numbers for unsigned types.
    // TODO: Add test for rejecting overflow for integer types.
    // TODO: Test operator>> on large negative numbers.
    // TODO: Figure out why ss >> std::hex; ss >> m_value behaves differently than ss >> std::hex >> m_value.

    struct Row
    {
        int m_int;
        double m_double;
        const char* m_text;
        bool m_bool;
        unsigned int m_unsignedInt;
        long long int m_longLong;
        unsigned long long int m_unsignedLongLong;
    };


    template <typename T>
    bool Expect(T expected, T observed)
    {
        if (expected != observed)
        {
            std::cout << "  incorrect value in data row - expected " << expected;
            std::cout << " but found " << observed << "." << std::endl;
            return false;
        }

        return true;
    }


    bool ValidateRoundtrip(ITableFormatter& formatter, ITableParser& parser, bool hexColumns)
    {
        bool success = true;

        Row rows[] = {
            {0xabcd, 1.0, "heLLo", true, 123, 1LL << 62, 1ULL << 63},
            {-1, -1.2, "string with embedded \"quote\"", false,
             123, 1LL << 62, 1ULL << 63},
            {1234, 0, "   string with 3 leading spaces", true, 123,
             1LL << 62, 1ULL << 63},
            {-6789, -1.2e-5, "string with comma (,)", false, 123,
             1LL << 62, 1ULL << 63},
            // TODO: decide whether to support embedded newlines.
            // Currently embedded newlines are not allowed in .TSV and .CSV files.
            //{5, 1234567.89, "string with newline (\n) embedded", false},
            {0, 10e6, "", true, 123, 1LL << 62, 1ULL << 63},
            {0x7fffffff, 1.0, "hexidecimal", true, 0xffffffff,
             0x7fffffffffffffff, 0xffffffffffffffff}  // Test hexidecimal values
        };

        InputColumn<int> c1("C1", "Integer column");
        InputColumn<double> c2("C2", "Double column");
        InputColumn<std::string> c3("C3", "String column");
        InputColumn<bool> c4("C4", "Boolean column");
        InputColumn<unsigned int> c5("C5", "Unsigned Integer column");
        InputColumn<long long int> c6("C6", "Long Long Integer column");
        InputColumn<unsigned long long int>
            c7("C7","Unsigned Long Long Integer column");

        c1.SetHexMode(hexColumns);
        c5.SetHexMode(hexColumns);
        c6.SetHexMode(hexColumns);
        c7.SetHexMode(hexColumns);

        TableWriter writer(formatter);
        writer.DefineColumn(c1);
        writer.DefineColumn(c2);
        writer.DefineColumn(c3);
        writer.DefineColumn(c4);
        writer.DefineColumn(c5);
        writer.DefineColumn(c6);
        writer.DefineColumn(c7);

        writer.WritePrologue();

        for (unsigned i = 0; i < sizeof(rows) / sizeof(Row); ++i)
        {
            c1.operator=(rows[i].m_int);
            c1 = rows[i].m_int;
            c2 = rows[i].m_double;
            c3 = rows[i].m_text;
            c4 = rows[i].m_bool;
            c5 = rows[i].m_unsignedInt;
            c6 = rows[i].m_longLong;
            c7 = rows[i].m_unsignedLongLong;

            writer.WriteDataRow();
        }

        writer.WriteEpilogue();

        TableReader reader(parser);
        reader.DefineColumn(c1);
        reader.DefineColumn(c2);
        reader.DefineColumn(c3);
        reader.DefineColumn(c4);
        reader.DefineColumn(c5);
        reader.DefineColumn(c6);
        reader.DefineColumn(c7);

        reader.ReadPrologue();

        for (unsigned i = 0; i < sizeof(rows)/sizeof(Row); ++i)
        {
            reader.ReadDataRow();

            if (c1 != rows[i].m_int ||
                c2 != rows[i].m_double ||
                strcmp(c3.GetValue().c_str(), rows[i].m_text) != 0 ||
                c4 != rows[i].m_bool ||
                c5 != rows[i].m_unsignedInt ||
                c6 != rows[i].m_longLong ||
                c7 != rows[i].m_unsignedLongLong)
            {
                std::cout << "Value mismatch on line " << i + 1 << "." << std::endl;
                success = false;
                break;
            }
        }

        if (success)
        {
            reader.ReadEpilogue();
        }

        return success;
    }


    bool ValidateTableRoundtrip(ITableFormatter* formatter, ITableParser& parser, bool hexColumns)
    {
        bool success = true;

        std::vector<int> v1;
        std::vector<unsigned> v2;
        std::vector<long long> v3;
        std::vector<unsigned long long> v4;
        std::vector<double> v5;
        std::vector<std::string> v6;
        std::vector<bool> v7;

        const unsigned c_rowCount = 2; //10;

        for (unsigned i = 0 ; i < c_rowCount; ++i)
        {
            v1.push_back(0 - i);
            v2.push_back(i);
            v3.push_back(i);
            v4.push_back(i);
            v5.push_back(i);
            std::ostringstream buffer;
            buffer << i;
            v6.push_back(buffer.str());
            v7.push_back((i % 2) == 0);
        }

        InputVectorColumn<int> c1("C1", "Integer column", v1);
        InputVectorColumn<unsigned int> c2("C2", "Unsigned Integer column", v2);
        InputVectorColumn<long long int> c3("C3", "Long Long Integer column", v3);
        InputVectorColumn<unsigned long long int> c4("C4", "Unsigned Long Long Integer column", v4);
        InputVectorColumn<double> c5("C5", "Double column", v5);
        InputVectorColumn<std::string> c6("C6", "String column", v6);
        InputVectorColumn<bool> c7("C7", "Boolean column", v7);

        c1.SetHexMode(hexColumns);
        c2.SetHexMode(hexColumns);
        c3.SetHexMode(hexColumns);
        c4.SetHexMode(hexColumns);

        VectorTable table;
        table.DefineColumn(c1);
        table.DefineColumn(c2);
        table.DefineColumn(c3);
        table.DefineColumn(c4);
        table.DefineColumn(c5);
        table.DefineColumn(c6);
        table.DefineColumn(c7);

        table.Write(*formatter);

        for (unsigned i = 0; i < c_rowCount; ++i)
        {
            c1.Clear();
            c2.Clear();
            c3.Clear();
            c4.Clear();
            c5.Clear();
            c6.Clear();
            c7.Clear();
        }

        table.Read(parser);

        for (unsigned i = 0 ; i < c_rowCount; ++i)
        {
            success &= Expect<int>(0 - i, v1[i]);
            success &= Expect<unsigned>(i, v2[i]);
            success &= Expect<long long>(i, v3[i]);
            success &= Expect<unsigned long long>(i, v4[i]);
            success &= Expect<double>(i, v5[i]);

            std::ostringstream buffer;
            buffer << i;
            if (v6[i] != buffer.str())
            {
                std::cout << "Value mismatch." << std::endl;
                success = false;
            }

            success &= Expect<bool>((i % 2) == 0, v7[i]);
        }

        return success;
    }


    TEST(CsvTsv, HeaderCommentMode)
    {
        std::string inputString("#foobar\n#barfoo\n#Fields:C1,C2,C3\n1,2.0,hello\n3,4.0,world\n");
        std::stringstream inputStream(inputString);

        InputColumn<int> c1("C1", "Integer column");
        InputColumn<double> c2("C2", "Double column");
        InputColumn<std::string> c3("C3", "String column");

        CsvTableParser parser(inputStream);
        parser.SetHeaderCommentMode(true);

        TableReader reader(parser);
        reader.DefineColumn(c1);
        reader.DefineColumn(c2);
        reader.DefineColumn(c3);

        reader.ReadPrologue();

        reader.ReadDataRow();
        ASSERT_TRUE(c1.GetValue() == 1 &&
                    c2.GetValue() == 2.0 &&
                    c3.GetValue().compare("hello") == 0);

        reader.ReadDataRow();
        ASSERT_TRUE(c1.GetValue() == 3 &&
                    c2.GetValue() == 4.0 &&
                    c3.GetValue().compare("world") == 0);
    }


    TEST(CsvTsv, GeneralComments)
    {
        std::string inputString =
            "#comment1\n"
            "#comment2\n"
            "C1\n"
            "#comment3\n"
            "1\n"
            "#comment4\n"
            "2\n"
            "\n"
            "3\n"
            "#comment5\n";

        std::stringstream inputStream(inputString);

        InputColumn<int> c1("C1", "Integer column");

        CsvTableParser parser(inputStream);

        TableReader reader(parser);
        reader.DefineColumn(c1);

        reader.ReadPrologue();

        // AtEOF() strips away comments while checking for EOF.
        ASSERT_TRUE(!reader.AtEOF());

        reader.ReadDataRow();
        ASSERT_TRUE(Expect<int>(1, c1));

        // AtEOF() strips away comments while checking for EOF.
        ASSERT_FALSE(reader.AtEOF());

        reader.ReadDataRow();
        ASSERT_TRUE(Expect<int>(2, c1));

        // AtEOF() strips away comments while checking for EOF.
        ASSERT_FALSE(reader.AtEOF());

        reader.ReadDataRow();
        ASSERT_TRUE(Expect<int>(3, c1));

        reader.ReadEpilogue();
    }


    TEST(CsvTsv, UnsignedLongInt)
    {
        const unsigned long int maximum =
            std::numeric_limits<unsigned long int>::max();
        const unsigned long int minimum =
            std::numeric_limits<unsigned long>::lowest();

        std::stringstream inputStream;
        inputStream << "C1" << std::endl
                    << "32" << std::endl
                    << "493434" << std::endl
                    << maximum << std::endl
                    << minimum << std::endl;
        // See bug#49.
        //          << "-1" << std::endl
        //          << "dummy_value_never_parsed" << std::endl;

        InputColumn<unsigned long int> c1("C1", "Integer column");

        CsvTableParser parser(inputStream);

        TableReader reader(parser);
        reader.DefineColumn(c1);

        reader.ReadPrologue();

        reader.ReadDataRow();
        ASSERT_EQ(c1.GetValue(), 32U);

        reader.ReadDataRow();
        ASSERT_EQ(c1.GetValue(), 493434U);

        reader.ReadDataRow();
        ASSERT_EQ(c1.GetValue(), maximum);

        reader.ReadDataRow();
        ASSERT_EQ(c1.GetValue(), minimum);

        // See bug#49.
        // Parsing the -1 causes ParserError.
        // ASSERT_THROW(reader.ReadDataRow(), CsvTsv::ParseError);

        // Parsing the dummy line correctly reads, but getting the value
        // delivers runtime_error.
        // reader.ReadDataRow();
        // ASSERT_THROW(c1.GetValue(), std::runtime_error);
    }


    TEST(CsvTsv, LongLongHex)
    {
        std::string inputString("C1\n001002F011F5\n0011FFB81230\n");
        std::stringstream inputStream(inputString);

        InputColumn<unsigned long long> c1("C1", "Integer column");
        c1.SetHexMode(true);

        CsvTableParser parser(inputStream);

        TableReader reader(parser);
        reader.DefineColumn(c1);

        reader.ReadPrologue();

        reader.ReadDataRow();
        ASSERT_EQ(c1.GetValue(), 0x001002F011F5ull);

        reader.ReadDataRow();
        ASSERT_EQ(c1.GetValue(), 0x0011FFB81230ull);
    }


    // This test verifies that the CSV parser can read a Bing manifest file
    // without crashing. It does not verify that the file was read correctly.
    // The only verification is counting the number of data rows read.
    TEST(CsvTsv, ReadBingManifestFile)
    {
        const std::string s_manifestFile =
            "#Version: 911\n"
            "#FormatVersion: 2\n"
            "#RankBegin: 479\n"
            "#RankEnd: 479\n"
            "#IBTrackMarker: 129650430289746635\n"
            "#ChunkClassDocCounts: HF:20577,HI:1618,II:33109,LF:107718,VI:3846,ZH:13284,ZL:14267,ZT:24979\n"
            "#SangamFlag: false\n"
            "#DataSourceNameID: 0\n"
            "#DataSourceVersion: 18446744073709551615\n"
            "#Timestamp: 11/06/2011 13:30:40\n"
            "#Fields:ChunkId,ChunkType,ChunkSubType,ChunkClass,RepublishVersion,ActivateVersion,RetireVersion,FullCRC,QuickCRC,QuickCRCType,DocCount,MachineList,SourcePath,RG\n"
            "001000D41482,index,0,LF,0,894,910,B80F80E82FD9B2D3,A0EB8E9944A2F44B,S,6232,CH1SCH010341909:CH1SCH010341910,001000D4\\chunk001000D41482.index,1of16DaysRG\n"
            "001000D41482,content,0,LF,0,894,910,BBCA586823604C73,EEC94B6A6D5C2E12,S,6232,CH1SCH010341909:CH1SCH010341910,001000D4\\chunk001000D41482.content,1of16DaysRG\n"
            "001000D41482,seenurl,0,LF,0,894,910,53EDAC2668FFC723,526A4A657E71354B,S,6232,CH1SCH010341909:CH1SCH010341910,001000D4\\chunk001000D41482.seenurl,1of16DaysRG\n"
            "001000D41482,dui,0,LF,16,909,910,5A800921C8E9E920,5A800921C8E9E920,S,6232,CH1SCH020052433:CH1SCH020052434,001000D4\\chunk001000D41482_v16.dui,1of16DaysRG\n"
            "001000D41597,index,0,II,0,911,-1,55F8B48F62633AF2,A8AE92C2B90EBF94,S,1038,CH1SCH020052433:CH1SCH020052434,001000D4\\chunk001000D41597.index,2of16DaysRG\n"
            "001000D41597,content,0,II,0,911,-1,5BEEE4538BF2075A,88A6CD126C314FA2,S,1038,CH1SCH020052433:CH1SCH020052434,001000D4\\chunk001000D41597.content,2of16DaysRG\n"
            "001000D41597,seenurl,0,II,0,911,-1,5EC582FC38360928,5EC582FC38360928,S,1038,CH1SCH020052433:CH1SCH020052434,001000D4\\chunk001000D41597.seenurl,2of16DaysRG\n"
            "001000D41598,index,0,VI,0,911,-1,CE14166B7BF128D9,38DCDA311D45E54D,S,117,CH1SCH020052433:CH1SCH020052434,001000D4\\chunk001000D41598.index,2of16DaysRG\n"
            "001000D41598,content,0,VI,0,911,-1,993EDDA04AB6C302,AB178F30D5440DF0,S,117,CH1SCH020052433:CH1SCH020052434,001000D4\\chunk001000D41598.content,2of16DaysRG\n"
            "001000D41598,seenurl,0,VI,0,911,-1,95013BE7682C19C5,95013BE7682C19C5,S,117,CH1SCH020052433:CH1SCH020052434,001000D4\\chunk001000D41598.seenurl,2of16DaysRG\n"
            "#EOF\n"
            "\n"
            "#CRC411C1D5D63DDE76A\n";

        std::stringstream input(s_manifestFile);

        // Preparse a Csv parser.
        CsvTsv::CsvTableParser parser(input);
        parser.SetHeaderCommentMode(true);

        CsvTsv::InputColumn<unsigned long long> chunkId("ChunkId", "ChunkId");
        chunkId.SetHexMode(true);
        CsvTsv::InputColumn<std::string> chunkType("ChunkType","ChunkType");
        CsvTsv::InputColumn<int> chunkSubType("ChunkSubType","ChunkSubType");
        CsvTsv::InputColumn<std::string> chunkClass("ChunkClass","unused");
        CsvTsv::InputColumn<std::string> republishVersion("RepublishVersion","unused");
        CsvTsv::InputColumn<unsigned> activateVersion("ActivateVersion","ActivateVersion");
        CsvTsv::InputColumn<int> retireVersion("RetireVersion","ActivateVersion");
        CsvTsv::InputColumn<std::string> fullCRC("FullCRC","unused");
        CsvTsv::InputColumn<std::string> quickCRC("QuickCRC","unused");
        CsvTsv::InputColumn<std::string> quickCRCType("QuickCRCType","unused");
        CsvTsv::InputColumn<int> docCount("DocCount","unused");
        CsvTsv::InputColumn<std::string> machineList("MachineList","unused");
        CsvTsv::InputColumn<std::string> sourcePath("SourcePath","path to chunk file");
        CsvTsv::InputColumn<std::string> rg("RG","unused");

        CsvTsv::TableReader reader(parser);
        reader.DefineColumn(chunkId);
        reader.DefineColumn(chunkType);
        reader.DefineColumn(chunkSubType);
        reader.DefineColumn(chunkClass);
        reader.DefineColumn(republishVersion);
        reader.DefineColumn(activateVersion);
        reader.DefineColumn(retireVersion);
        reader.DefineColumn(fullCRC);
        reader.DefineColumn(quickCRC);
        reader.DefineColumn(quickCRCType);
        reader.DefineColumn(docCount);
        reader.DefineColumn(machineList);
        reader.DefineColumn(sourcePath);
        reader.DefineColumn(rg);

        reader.ReadPrologue();

        // Read in the file, organizing chunks with the correct version into ChunkDescriptors.
        unsigned rowCount = 0;
        while (!reader.AtEOF())
        {
            reader.ReadDataRow();
            ++rowCount;
        }

        ASSERT_EQ(rowCount, 10u);
    }


    TEST(CsvTsv, Roundtripping)
    {
        std::stringstream csv1;
        CsvTableFormatter csvFormatter1(csv1);
        CsvTableParser csvParser1(csv1);
        ASSERT_TRUE(ValidateRoundtrip(csvFormatter1, csvParser1, false));

        std::stringstream csv2;
        CsvTableFormatter csvFormatter2(csv2);
        CsvTableParser csvParser2(csv2);
        ASSERT_TRUE(ValidateRoundtrip(csvFormatter2, csvParser2, true));

        std::stringstream csv3;
        CsvTableFormatter csvFormatter3(csv3);
        CsvTableParser csvParser3(csv3);
        csvFormatter3.SetHeaderCommentMode(true);
        csvParser3.SetHeaderCommentMode(true);
        ASSERT_TRUE(ValidateRoundtrip(csvFormatter3, csvParser3, true));

        std::stringstream tsv;
        TsvTableFormatter tsvFormatter(tsv);
        TsvTableParser tsvParser(tsv);
        ASSERT_TRUE(ValidateRoundtrip(tsvFormatter, tsvParser, false));

        std::stringstream csv4;
        CsvTableFormatter csvFormatter4(csv4);
        CsvTableParser csvParser4(csv4);
        ASSERT_TRUE(ValidateTableRoundtrip(&csvFormatter4, csvParser4, false));

        std::stringstream csv5;
        CsvTableFormatter csvFormatter5(csv5);
        CsvTableParser csvParser5(csv5);
        ASSERT_TRUE(ValidateTableRoundtrip(&csvFormatter5, csvParser5, true));

        std::stringstream csv6;
        CsvTableFormatter csvFormatter6(csv6);
        CsvTableParser csvParser6(csv6);
        csvFormatter6.SetHeaderCommentMode(true);
        csvParser6.SetHeaderCommentMode(true);
        ASSERT_TRUE(ValidateTableRoundtrip(&csvFormatter6, csvParser6, true));
    }
}
