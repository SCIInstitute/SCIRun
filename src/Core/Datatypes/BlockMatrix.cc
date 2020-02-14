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


#include <Core/Datatypes/BlockMatrix.h>
#include <numeric>
#include <algorithm>

using namespace SCIRun::Core::Datatypes;

DenseBlockMatrix::DenseBlockMatrix(const RowBlockSizes& rbs, const ColumnBlockSizes& cbs) :
  blockStartsRow_(rbs.size() + 1),
  blockStartsCol_(cbs.size() + 1)
{
  const int totalRows = std::accumulate(rbs.begin(), rbs.end(), 0);
  std::partial_sum(rbs.begin(), rbs.end(), blockStartsRow_.begin() + 1);

  const int totalCols = std::accumulate(cbs.begin(), cbs.end(), 0);
  std::partial_sum(cbs.begin(), cbs.end(), blockStartsCol_.begin() + 1);

  matrix_.resize(totalRows, totalCols);
  matrix_.setZero();
}

int DenseBlockMatrix::startRow(int i) const
{
  return blockStartsRow_[i];
}

int DenseBlockMatrix::startCol(int j) const
{
  return blockStartsCol_[j];
}

int DenseBlockMatrix::rowSize(int i) const
{
  return blockStartsRow_[i+1] - blockStartsRow_[i];
}

int DenseBlockMatrix::colSize(int j) const
{
  return blockStartsCol_[j+1] - blockStartsCol_[j];
}
