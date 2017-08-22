/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Utils/Exception.h>
#include <numeric>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;

SparseRowMatrixHandle SparseRowMatrixFromMap::make(size_type rows, size_type cols, const SparseRowMatrixFromMap::Values& data)
{
  typedef Eigen::Triplet<double> T;
  std::vector<T> tripletList;
  tripletList.reserve(get_nnz(data));

  for (auto row = data.begin(); row != data.end(); ++row)
  {
    auto rowIndex = row->first;
    for (auto colVal = row->second.begin(); colVal != row->second.end(); ++colVal)
    {
      tripletList.push_back(T(rowIndex, colVal->first, colVal->second));
    }
  }
  SparseRowMatrixHandle mat(boost::make_shared<SparseRowMatrix>(rows,cols));
  mat->setZero();
  mat->setFromTriplets(tripletList.begin(), tripletList.end());
  return mat;
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

SparseRowMatrixHandle SparseRowMatrixFromMap::appendToSparseMatrix(size_type rows, size_type cols, const SparseRowMatrix& sparse, const Values& additionalValues)
{
  if (rows < sparse.nrows() || cols < sparse.ncols())
    THROW_INVALID_ARGUMENT("new matrix needs to be at least the size of old matrix");

  const size_type nnz = get_nnz(additionalValues) + sparse.nonZeros();

  typedef Eigen::Triplet<double> T;
  std::vector<T> tripletList;
  tripletList.reserve(nnz);

  for (int k=0; k < sparse.outerSize(); ++k)
  {
    for (Core::Datatypes::SparseRowMatrix::InnerIterator it(sparse,k); it; ++it)
    {
      auto row = additionalValues.find(it.row());
      if (row != additionalValues.end())
      {
        auto col = row->second.find(it.col());
        if (col == row->second.end())
        {
          tripletList.push_back(T(it.row(), it.col(), it.value()));
        }
      }
      else
        tripletList.push_back(T(it.row(), it.col(), it.value()));
    }
  }

  for (auto row = additionalValues.begin(); row != additionalValues.end(); ++row)
  {
    auto rowIndex = row->first;
    for (auto colVal = row->second.begin(); colVal != row->second.end(); ++colVal)
    {
      tripletList.push_back(T(rowIndex, colVal->first, colVal->second));
    }
  }
  SparseRowMatrixHandle mat(boost::make_shared<SparseRowMatrix>(rows, cols));
  mat->setFromTriplets(tripletList.begin(), tripletList.end());

  return mat;
}

SparseRowMatrixHandle SparseRowMatrixFromMap::appendToSparseMatrixSumming(size_type rows, size_type cols, const SparseRowMatrix& sparse, const Values& additionalValues)
{
  if (rows < sparse.nrows() || cols < sparse.ncols())
    THROW_INVALID_ARGUMENT("new matrix needs to be at least the size of old matrix");

  const size_type nnz = get_nnz(additionalValues) + sparse.nonZeros();

  typedef Eigen::Triplet<double> T;
  std::vector<T> tripletList;
  tripletList.reserve(nnz);

  for (auto row = additionalValues.begin(); row != additionalValues.end(); ++row)
  {
    auto rowIndex = row->first;
    for (auto colVal = row->second.begin(); colVal != row->second.end(); ++colVal)
    {
      tripletList.push_back(T(rowIndex, colVal->first, colVal->second));
    }
  }
  SparseRowMatrixHandle mat(boost::make_shared<SparseRowMatrix>(rows, cols));
  mat->setFromTriplets(tripletList.begin(), tripletList.end());

  SparseRowMatrix empty(rows, cols);
  SparseRowMatrix originalValuesLarger = sparse + empty;
  (*mat) += originalValuesLarger;
  return mat;
}


SparseRowMatrixHandle SparseRowMatrixFromMap::concatenateSparseMatrices(const SparseRowMatrix& mat1, const SparseRowMatrix& mat2, const bool rows)
{
 SparseRowMatrixHandle mat;
 size_type offset_rows=0,offset_cols=0;
 
 if ( (rows && mat1.ncols() != mat2.ncols()) || (!rows && mat1.nrows() != mat2.nrows()) ) 
   THROW_INVALID_ARGUMENT(" Matrix dimensions do not match! ");
 
 const size_type nnz = mat1.nonZeros() + mat2.nonZeros();

 typedef Eigen::Triplet<double> T;
 std::vector<T> tripletList;
 tripletList.reserve(nnz);
  for (size_type k=0; k < mat1.outerSize(); ++k)
  {
    for (Core::Datatypes::SparseRowMatrix::InnerIterator it(mat1,k); it; ++it)
    {
      tripletList.push_back(T(it.row(), it.col(), it.value()));
    }
  }
  
  if (rows)
  {
   offset_rows=mat1.nrows();
   offset_cols=0;
  } else
  {
   offset_rows=0;
   offset_cols=mat1.ncols();
  }
  
  for (size_type k=0; k < mat2.outerSize(); ++k)
  {
    for (Core::Datatypes::SparseRowMatrix::InnerIterator it(mat2,k); it; ++it)
    {
      tripletList.push_back(T(it.row()+offset_rows, it.col()+offset_cols, it.value()));
    }
  }
  
  if(rows)
  {
    mat=boost::make_shared<SparseRowMatrix>(mat1.nrows()+mat2.nrows(),mat1.ncols());
  } else
  {
    mat=boost::make_shared<SparseRowMatrix>(mat1.nrows(),mat1.ncols()+mat2.ncols());
  }
  
  mat->setFromTriplets(tripletList.begin(), tripletList.end());
    
 return mat;
}
