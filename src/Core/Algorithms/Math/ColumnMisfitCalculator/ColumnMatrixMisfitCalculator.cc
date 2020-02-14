/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Algorithms/Math/ColumnMisfitCalculator/ColumnMatrixMisfitCalculator.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

ALGORITHM_PARAMETER_DEF(Math, MisfitMethod);
ALGORITHM_PARAMETER_DEF(Math, PValue);
ALGORITHM_PARAMETER_DEF(Math, ccInv);
ALGORITHM_PARAMETER_DEF(Math, rmsRel);

ColumnMatrixMisfitCalculator::ColumnMatrixMisfitCalculator(const DenseColumnMatrix& x, const DenseColumnMatrix& y, double pp)
: rmsRel_(0), rms_(0), cc_(0), ccInv_(0), pp_(pp)
{
  if (x.nrows() != y.nrows())
    return;

  size_t ne = y.nrows();

  // compute CC

  double avg1 = 0, avg2 = 0;
  for (int i = 0; i < ne; ++i)
  {
    avg1 += x[i];
    avg2 += y[i];
  }
  avg1 /= ne;
  avg2 /= ne;

  double Norm1 = 0;
  double ccNum = 0;
  double ccDenom1 = 0;
  double ccDenom2 = 0;
  double rms = 0;
  for (int i = 0; i < ne; ++i)
  {
    double shift1 = (x[i]-avg1);
    double shift2 = (y[i]-avg2);

    ccNum += shift1 * shift2;
    ccDenom1 += shift1 * shift1;
    ccDenom2 += shift2 * shift2;
    double tmp = fabs(x[i]-y[i]);
    if (pp == 1)
    {
      rms += tmp;
      Norm1 += fabs(x[i]);
    }
    else if (pp == 2)
    {
      rms += tmp * tmp;
      Norm1 += (x[i])*(y[i]);
    }
    else
    {
      rms += pow(tmp,pp);
      Norm1 += pow(fabs(x[i]),pp);
    }
  }

  rms_ = pow(rms/ne,1/pp);
  double ccDenom = std::sqrt(ccDenom1*ccDenom2);

  const double MAX_VALUE = 1e6;
  cc_ = std::min(ccNum/ccDenom, MAX_VALUE);
  ccInv_ = std::min(1.0 - ccNum / ccDenom, MAX_VALUE);
  rmsRel_ = std::min(rms*pow(ne / Norm1, 1 / pp), MAX_VALUE);
}

double ColumnMatrixMisfitCalculator::getCorrelationCoefficient() const
{
  return cc_;
}

double ColumnMatrixMisfitCalculator::getInverseCorrelationCoefficient() const
{
  return ccInv_;
}

double ColumnMatrixMisfitCalculator::getRelativeRMS() const
{
  return rmsRel_;
}

double ColumnMatrixMisfitCalculator::getRMS() const
{
  return rms_;
}

double ColumnMatrixMisfitCalculator::getPValue() const
{
  return pp_;
}
