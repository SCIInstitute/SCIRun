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


#ifndef CORE_DATATYPES_SPARSEMATRIXFROMMAP_H
#define CORE_DATATYPES_SPARSEMATRIXFROMMAP_H

#include <map>
#include <boost/shared_array.hpp>
#include <boost/make_shared.hpp>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Utils/Exception.h>
#include <numeric>
#include <Core/Datatypes/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Datatypes
    {
      /// @todo: refactor with some OO, please...

      template <typename T>
      class SparseRowMatrixFromMapGeneric
      {
      public:
        using Row = std::map<index_type, T>;
        using Values = std::map<index_type, Row>;

        class SCISHARE SymmetricValues
        {
        public:
          void operator()(size_t row, size_t col, T value)
          {
            values_[row][col] = value;
            values_[col][row] = value;
          }
          const Values& getFullValues() const { return values_; }
        private:
          Values values_;
        };

        static SharedPointer<SparseRowMatrixGeneric<T>> make(size_type rows, size_type cols, const Values& values)
        {
          using Triplet = Eigen::Triplet<T>;
          std::vector<Triplet> tripletList;
          tripletList.reserve(get_nnz(values));

          for (auto row = values.begin(); row != values.end(); ++row)
          {
            auto rowIndex = row->first;
            for (auto colVal = row->second.begin(); colVal != row->second.end(); ++colVal)
            {
              tripletList.push_back(Triplet(rowIndex, colVal->first, colVal->second));
            }
          }
          auto mat(boost::make_shared<SparseRowMatrixGeneric<T>>(rows, cols));
          mat->setZero();
          mat->setFromTriplets(tripletList.begin(), tripletList.end());
          return mat;
        }

        static SharedPointer<SparseRowMatrixGeneric<T>> make(size_type rows, size_type cols, const SymmetricValues& values)
        {
          return make(rows, cols, values.getFullValues());
        }

        static SharedPointer<SparseRowMatrixGeneric<T>> appendToSparseMatrix(size_type rows, size_type cols, const SparseRowMatrixGeneric<T>& sparse, const Values& additionalValues)
        {
          if (rows < sparse.nrows() || cols < sparse.ncols())
            THROW_INVALID_ARGUMENT("new matrix needs to be at least the size of old matrix");

          const size_type nnz = get_nnz(additionalValues) + sparse.nonZeros();

          using Triplet = Eigen::Triplet<T>;
          std::vector<Triplet> tripletList;
          tripletList.reserve(nnz);

          for (int k = 0; k < sparse.outerSize(); ++k)
          {
            for (typename SparseRowMatrixGeneric<T>::InnerIterator it(sparse, k); it; ++it)
            {
              auto row = additionalValues.find(it.row());
              if (row != additionalValues.end())
              {
                auto col = row->second.find(it.col());
                if (col == row->second.end())
                {
                  tripletList.push_back(Triplet(it.row(), it.col(), it.value()));
                }
              }
              else
                tripletList.push_back(Triplet(it.row(), it.col(), it.value()));
            }
          }

          for (auto row = additionalValues.begin(); row != additionalValues.end(); ++row)
          {
            auto rowIndex = row->first;
            for (auto colVal = row->second.begin(); colVal != row->second.end(); ++colVal)
            {
              tripletList.push_back(Triplet(rowIndex, colVal->first, colVal->second));
            }
          }
          auto mat(boost::make_shared<SparseRowMatrixGeneric<T>>(rows, cols));
          mat->setFromTriplets(tripletList.begin(), tripletList.end());

          return mat;
        }

        static SharedPointer<SparseRowMatrixGeneric<T>> appendToSparseMatrixSumming(size_type rows, size_type cols, const SparseRowMatrixGeneric<T>& sparse, const Values& additionalValues)
        {
          if (rows < sparse.nrows() || cols < sparse.ncols())
            THROW_INVALID_ARGUMENT("new matrix needs to be at least the size of old matrix");

          const size_type nnz = get_nnz(additionalValues) + sparse.nonZeros();

          using Triplet = Eigen::Triplet<T>;
          std::vector<Triplet> tripletList;
          tripletList.reserve(nnz);

          for (auto row = additionalValues.begin(); row != additionalValues.end(); ++row)
          {
            auto rowIndex = row->first;
            for (auto colVal = row->second.begin(); colVal != row->second.end(); ++colVal)
            {
              tripletList.push_back(Triplet(rowIndex, colVal->first, colVal->second));
            }
          }
          auto mat(boost::make_shared<SparseRowMatrixGeneric<T>>(rows, cols));
          mat->setFromTriplets(tripletList.begin(), tripletList.end());

          SparseRowMatrixGeneric<T> empty(rows, cols);
          SparseRowMatrixGeneric<T> originalValuesLarger = sparse + empty;
          (*mat) += originalValuesLarger;
          return mat;
        }

        static SparseRowMatrixHandle concatenateSparseMatrices(const SparseRowMatrix& mat1, const SparseRowMatrix& mat2, const bool rows)
        {
          size_type offset_rows = 0, offset_cols = 0;

          if ((rows && mat1.ncols() != mat2.ncols()) || (!rows && mat1.nrows() != mat2.nrows()))
            THROW_INVALID_ARGUMENT(" Matrix dimensions do not match! ");

          const size_type nnz = mat1.nonZeros() + mat2.nonZeros();

          using Triplet = Eigen::Triplet<T>;
          std::vector<Triplet> tripletList;
          tripletList.reserve(nnz);
          for (size_type k = 0; k < mat1.outerSize(); ++k)
          {
            for (typename SparseRowMatrixGeneric<T>::InnerIterator it(mat1, k); it; ++it)
            {
              tripletList.push_back(Triplet(it.row(), it.col(), it.value()));
            }
          }

          if (rows)
          {
            offset_rows = mat1.nrows();
            offset_cols = 0;
          }
          else
          {
            offset_rows = 0;
            offset_cols = mat1.ncols();
          }

          for (size_type k = 0; k < mat2.outerSize(); ++k)
          {
            for (typename SparseRowMatrixGeneric<T>::InnerIterator it(mat2, k); it; ++it)
            {
              tripletList.push_back(Triplet(it.row() + offset_rows, it.col() + offset_cols, it.value()));
            }
          }

          SharedPointer<SparseRowMatrixGeneric<T>> mat;
          if (rows)
          {
            mat = boost::make_shared<SparseRowMatrixGeneric<T>>(mat1.nrows() + mat2.nrows(), mat1.ncols());
          }
          else
          {
            mat = boost::make_shared<SparseRowMatrixGeneric<T>>(mat1.nrows(), mat1.ncols() + mat2.ncols());
          }

          mat->setFromTriplets(tripletList.begin(), tripletList.end());

          return mat;
        }
      private:
        SparseRowMatrixFromMapGeneric() = delete;

        struct SizeOfSecond
        {
          template <class Pair>
          size_t operator()(size_t sum, const Pair& p) const
          {
            return sum + p.second.size();
          }
        };

        static size_type get_nnz(const Values& data)
        {
          return std::accumulate(data.begin(), data.end(), static_cast<size_t>(0), SizeOfSecond());
        }
      };

      using SparseRowMatrixFromMap = SparseRowMatrixFromMapGeneric<double>;

      template <typename T>
      class LegacySparseDataContainer
      {
      public:
        using Rows = boost::shared_array<index_type>;
        using Columns = boost::shared_array<index_type>;
        using Storage = boost::shared_array<T>;

        LegacySparseDataContainer(size_type rowSize, size_type columnSize, size_type dataSize) :
          rows_(new index_type[rowSize]),
          columns_(new index_type[columnSize]),
          data_(new T[dataSize])
        { }
        LegacySparseDataContainer(size_type rowSize, size_type columnAndDataSize) :
          rows_(new index_type[rowSize]),
          columns_(new index_type[columnAndDataSize]),
          data_(new T[columnAndDataSize])
        { }

        bool allocated() const { return rows_ && columns_ && data_; }

        const Rows& rows() const { return rows_; }
        const Columns& columns() const { return columns_; }
        const Storage& data() const { return data_; }

        void allocateData(size_type n) { data_.reset(new T[n]); }

      private:
        const Rows rows_;
        const Columns columns_;
        Storage data_;
      };
    }
  }
}

#endif
