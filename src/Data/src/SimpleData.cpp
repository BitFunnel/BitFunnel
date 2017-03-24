// The MIT License (MIT)
//
// Copyright (c) 2016, Microsoft
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "BitFunnel/Data/Sonnets.h"


namespace BitFunnel
{

    //**********************************************************
    //
    // TODO
    //
    //**********************************************************
    namespace SimpleData
    {
        const char chunk0[] =
            "0000000000000001\0"
            "01\0SimpleData\0" "1\0\0"
            "00\0"
            "zero\0"
            "\0\0"
            "0000000000000002\0"
            "01\0SimpleData\0" "2\0\0"
            "00\0"
            "zero\0one\0"
            "\0\0"
            "0000000000000003\0"
            "01\0SimpleData\0" "3\0\0"
            "00\0"
            "zero\0one\0two\0"
            "\0\0"
            "0000000000000004\0"
            "01\0SimpleData\0" "4\0\0"
            "00\0"
            "zero\0one\0two\0three\0"
            "\0\0";

        const char chunk1[] =
            "0000000000000005\0"
            "01\0SimpleData\0" "5\0\0"
            "00\0"
            "zero\0one\0two\0three\0four\0"
            "\0\0"
            "0000000000000006\0"
            "01\0SimpleData\0" "6\0\0"
            "00\0"
            "zero\0one\0two\0three\0four\0five\0"
            "\0\0"
            "0000000000000007\0"
            "01\0SimpleData\0" "7\0\0"
            "00\0"
            "zero\0one\0two\0three\0four\0five\0six\0"
            "\0\0"
            "0000000000000008\0"
            "01\0SimpleData\0" "8\0\0"
            "00\0"
            "zero\0one\0two\0three\0four\0five\0six\0seven\0"
            "\0\0";



        std::vector<std::pair<size_t, char const *>> chunks =
        {
            std::make_pair(sizeof(chunk0), chunk0),
            std::make_pair(sizeof(chunk1), chunk1),
        };
    }
}
