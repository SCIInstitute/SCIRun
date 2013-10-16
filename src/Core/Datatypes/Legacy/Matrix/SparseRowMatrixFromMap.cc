/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/Legacy/Matrix/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <numeric>
#include <stdexcept>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;

#if 0
SparseRowMatrixHandle SparseRowMatrixFromMap::make(size_type rows, size_type cols, const SparseRowMatrixFromMap::Values& data)
{
  size_type nnz = get_nnz(data);
  SparseRowMatrix::Data sparseData(rows + 1, nnz);

  const SparseRowMatrix::Rows& rr = sparseData.rows();
  const SparseRowMatrix::Columns& cc = sparseData.columns();
  const SparseRowMatrix::Storage& vv = sparseData.data();

  SparseRowMatrix::Columns::element_type* columnIterator = cc.get();
  SparseRowMatrix::Storage::element_type* valueIterator = vv.get();

  std::vector<size_t> numNonZeroPerRow(rows);
  for (size_type r = 0; r < rows; ++r)
  {
    size_type rowIndex = r;
    const Values::const_iterator rowPtr = data.find(r);
    if (rowPtr != data.end())
    {
      const Values::value_type::second_type& rowData = rowPtr->second;
      numNonZeroPerRow[rowIndex] = rowData.size();
      for (Values::value_type::second_type::const_iterator c = rowData.begin(); c != rowData.end(); ++c)
      {
        *columnIterator++ = c->first;
        *valueIterator++ = c->second;
      }  
    }
  }

  rr[0] = 0;
  std::partial_sum(numNonZeroPerRow.begin(), numNonZeroPerRow.end(), rr.get() + 1);

  SparseRowMatrixHandle matrix(new SparseRowMatrix(rows, cols, sparseData, nnz));
  return matrix;
}

struct SizeOfSecond
{
  template <class Pair>
  size_t operator()(size_t sum, const Pair& p) const
  {
    return sum + p.second.size();
  }
};

size_type SparseRowMatrixFromMap::get_nnz(const Values& data)
{
  return std::accumulate(data.begin(), data.end(), (size_t)0, SizeOfSecond());
}

SparseRowMatrixHandle SparseRowMatrixFromMap::make(size_type rows, size_type cols, const SymmetricValues& data)
{
  return make(rows, cols, data.getFullValues());
}

void SparseRowMatrixFromMap::SymmetricValues::operator()(size_t row, size_t col, double value)
{
  values_[row][col] = value;
  values_[col][row] = value;
}

const SparseRowMatrixFromMap::Values& SparseRowMatrixFromMap::SymmetricValues::getFullValues() const
{
  return values_;
}
#endif

SparseRowMatrixHandle SparseRowMatrixFromMap::appendToSparseMatrix(size_type rows, size_type cols, const SparseRowMatrix& sparse, const Values& additionalValues)
{
  //TODO : change to boost exception
  if (rows < sparse.nrows() || cols < sparse.ncols())
    throw std::invalid_argument("new matrix needs to be at least the size of old matrix");

  return SparseRowMatrixHandle();

#if 0
  const size_type nnz = get_nnz(additionalValues) + sparse.get_nnz();
  SparseRowMatrix::Data newSparseData(rows + 1, nnz);

  const SparseRowMatrix::Rows& new_rr = newSparseData.rows();
  const SparseRowMatrix::Columns& new_cc = newSparseData.columns();
  const SparseRowMatrix::Storage& new_vv = newSparseData.data();

  const index_type* old_rr = sparse.get_rows();
  const index_type* old_cc = sparse.get_cols();
  const double* old_vv = sparse.get_vals();

  SparseRowMatrix::Columns::element_type* columnIterator = new_cc.get();
  SparseRowMatrix::Storage::element_type* valueIterator = new_vv.get();

  std::vector<size_t> numNonZeroPerRow(rows);
  for (size_type r = 0; r < rows; ++r)
  {
    size_type rowIndex = r;

    if (r < sparse.nrows())
    {
      size_type numNonZeroInOldRowI = old_rr[r+1] - old_rr[r];
      if (numNonZeroInOldRowI != 0)
      {
        numNonZeroPerRow[rowIndex] += numNonZeroInOldRowI;

        for (int c = 0; c < numNonZeroInOldRowI; ++c)
        {
          *columnIterator++ = *old_cc++;
          *valueIterator++ = *old_vv++;
        }  
      }
    }

    const Values::const_iterator rowPtr = additionalValues.find(r);
    if (rowPtr != additionalValues.end())
    {
      const Row& rowData = rowPtr->second;
      numNonZeroPerRow[rowIndex] += rowData.size();
      for (Row::const_iterator c = rowData.begin(); c != rowData.end(); ++c)
      {
        *columnIterator++ = c->first;
        *valueIterator++ = c->second;
      }  
    }
  }

  new_rr[0] = 0;
  std::partial_sum(numNonZeroPerRow.begin(), numNonZeroPerRow.end(), new_rr.get() + 1);

  SparseRowMatrixHandle matrix(new SparseRowMatrix(rows, cols, newSparseData, nnz));
  return matrix;
#endif
}
