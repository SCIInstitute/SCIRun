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


#include <Core/Math/TrigTable.h>
#include <Core/Math/MiscMath.h>
#include <Core/Exceptions/AssertionFailed.h>

namespace SCIRun {

SinCosTable::SinCosTable() : n_(0)
{
}

SinCosTable::SinCosTable(int n, double min, double max, double scale)
: n_(n), sindata_(n), cosdata_(n)
{
  if (n == 1)
    throw AssertionFailed("Invalid table size of 1", __FILE__, __LINE__);
  fill_table(max, min, scale);
}

void
SinCosTable::build_table(int n, double min, double max, double scale)
{
  sindata_.clear();
  cosdata_.clear();

  n_ = n;
  sindata_.resize(n_);
  cosdata_.resize(n_);
  fill_table(max, min, scale);
}

void SinCosTable::fill_table( double max, double min, double scale )
{
  /// @todo: white space, casting
  double d=max-min;
  for(int i=0;i<n_;i++)
  {
    double th=d*double(i)/double(n_-1)+min;
    sindata_[i] = std::sin(th)*scale;
    cosdata_[i] = std::cos(th)*scale;
  }
}

} // end namespace
