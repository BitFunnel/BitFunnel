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

#include <vector>                           // std::vector member.

#include "BitFunnel/Index/ITermTreatment.h" // Base class.
#include "OptimalTermTreatments.h"  // Temporary, for TermTreatmentMetrics.

namespace BitFunnel
{
    TermTreatmentMetrics AnalyzeAlternate(std::vector<int> rows,
                                          double density,
                                          double signal);


    size_t SizeTFromRowVector(std::vector<int> rows);


    //*************************************************************************
    //
    // TreatmentPrivateRank0
    //
    // Every term gets the same treatment: a single, private, rank 0 row.
    //
    //*************************************************************************
    class TreatmentPrivateRank0 : public ITermTreatment
    {
    public:
        TreatmentPrivateRank0(double density, double snr, int variant);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;


        //
        // Static methods used by ITermTreatmentFactory
        //
        static char const * GetName()
        {
            return "PrivateRank0";
        }


        static char const * GetDescription()
        {
            return "Each term gets a single, private rank 0 row.";
        }

    private:
        RowConfiguration m_configuration;
    };


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0
    //
    // Term treatment based on target bit density, signal to noise ratio and
    // the term's IdfSum() value.
    //
    // Treatments only involve rank 0 rows, but common terms will get a private
    // row, while rare terms will get multiple shared rows.
    //
    //*************************************************************************
    class TreatmentPrivateSharedRank0 : public ITermTreatment
    {
    public:
        TreatmentPrivateSharedRank0(double density, double snr, int variant);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;


        //
        // Static methods used by ITermTreatmentFactory
        //
        static char const * GetName()
        {
            return "PrivateSharedRank0";
        }


        static char const * GetDescription()
        {
            return "Each term gets either a private rank 0 row or a set of shared rank 0 rows.";
        }

    private:
        std::vector<RowConfiguration> m_configurations;
    };


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0And3
    //
    // Term treatment based on target bit density, signal to noise ratio and
    // the term's IdfSum() value.
    //
    // Treatments may now include rank 0 and rank 3 rows. Common terms will
    // get a private row, while rare terms will get a combination of rank of
    // private and shared rows from ranks 0 and 3.
    //
    //*************************************************************************
    class TreatmentPrivateSharedRank0And3 : public ITermTreatment
    {
    public:
        TreatmentPrivateSharedRank0And3(double density, double snr, int variant);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;


        //
        // Static methods used by ITermTreatmentFactory
        //
        static char const * GetName()
        {
            return "PrivateSharedRank0And3";
        }


        static char const * GetDescription()
        {
            return "Each term gets either a private rank 0 row or or two shared rank 0 rows and a zero or more rank 3 rows.";
        }

    private:
        std::vector<RowConfiguration> m_configurations;
    };


    class TreatmentPrivateSharedRank0ToN : public ITermTreatment
    {
    public:
        TreatmentPrivateSharedRank0ToN(double density, double snr, int variant);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;


        //
        // Static methods used by ITermTreatmentFactory
        //
        static char const * GetName()
        {
            return "PrivateSharedRank0ToN";
        }


        static char const * GetDescription()
        {
            return "Each term gets either a private rank 0 row or or two shared rank 0 rows and a zero or more higher rank rows.";
        }

    private:
        std::vector<RowConfiguration> m_configurations;
    };


    // TODO: remove prototype.
    std::pair<double, std::vector<int>> Temp(double frequency, double density, double snr, int currentRank, std::vector<int> rows, int maxRowsPerRank);

    class TreatmentExperimental : public ITermTreatment
    {
    public:
        TreatmentExperimental(double density, double snr, int variant);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;


        //
        // Static methods used by ITermTreatmentFactory
        //
        static char const * GetName()
        {
            return "Experimental";
        }


        static char const * GetDescription()
        {
            return "Experimental flavor of the day.";
        }

    private:
        std::vector<RowConfiguration> m_configurations;
    };


    class TreatmentOptimal : public ITermTreatment
    {
    public:
        TreatmentOptimal(double density, double snr, int variant);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;


        //
        // Static methods used by ITermTreatmentFactory
        //
        static char const * GetName()
        {
            return "Optimal";
        }


        static char const * GetDescription()
        {
            return "Optimal TermTreatment, given some assumptions.";
        }

    private:
        std::vector<RowConfiguration> m_configurations;
    };
}
