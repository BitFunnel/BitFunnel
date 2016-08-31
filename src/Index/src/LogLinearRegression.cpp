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

#include <cmath>

#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "LogLinearRegression.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // LogLinearRegression
    //
    //*************************************************************************
    LogLinearRegression::LogLinearRegression()
      : m_count(0),
        m_sumX(0),
        m_sumY(0),
        m_sumXX(0),
        m_sumXY(0),
        m_slope(0),
        m_intercept(0)
    {
    }


    void LogLinearRegression::AddPoint(double xOriginal, double yOriginal)
    {
        //double x = std::log(xOriginal);
        //double y = yOriginal;
        //m_count++;
        //m_sumX += x;
        //m_sumY += y;
        //m_sumXX += (x * x);
        //m_sumXY += (x * y);
        double x = xOriginal;
        double y = 1.0 / yOriginal;
        m_count++;
        m_sumX += x;
        m_sumY += y;
        m_sumXX += (x * x);
        m_sumXY += (x * y);
    }


    void LogLinearRegression::FitCurve()
    {
        m_slope = (m_count * m_sumXY - m_sumX * m_sumY) / (m_count * m_sumXX - m_sumX * m_sumX);
        m_intercept = (m_sumY - m_slope * m_sumX) / m_count;
    }


    void LogLinearRegression::FitCurve2(IDocumentFrequencyTable const & dft)
    {
        size_t lastX = dft.size() - 1;
        size_t firstX = lastX;
        size_t x = lastX;
        double lastY = dft[lastX].GetFrequency();

        for (; x != 0; --x)
        {
            if (dft[x].GetFrequency() > lastY)
            {
                break;
            }
            else
            {
                firstX = x;
            }
        }

        // Convert x to start at 1 instead of 0.
        double x2 = (lastX + 1.0 + firstX + 1.0) / 2.0;
        double y2 = 1.0/ lastY;

        double x1 = 1;
        double y1 = 1.0 / dft[0].GetFrequency();

        m_slope = (y2 - y1) / (x2 - x1);
        m_intercept = y1 - m_slope * x1;
    }


    double LogLinearRegression::Value(double xOriginal)
    {
        //double x = std::log(xOriginal);
         //double y = m_slope * x + m_intercept;
        //double yFinal = y;
        //return yFinal;
        double x = xOriginal;
        double y = m_slope * x + m_intercept;
        double yFinal = 1.0 / y;
        return yFinal;
    }
}
