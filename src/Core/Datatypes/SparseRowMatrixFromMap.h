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
/// @todo Documentation Core/Datatypes/SparceRowMatrixFromMap.h

#ifndef CORE_DATATYPES_SPARSEMATRIXFROMMAP_H
#define CORE_DATATYPES_SPARSEMATRIXFROMMAP_H

#include <map>
#include <boost/shared_array.hpp>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Datatypes/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Datatypes
    {
      /// @todo: refactor with some OO, please...
      class SCISHARE SparseRowMatrixFromMap
      {
      public:
        typedef std::map<index_type, double> Row;
        typedef std::map<index_type, Row> Values;

        class SCISHARE SymmetricValues
        {
        public:
          void operator()(size_t row, size_t col, double value);
          const Values& getFullValues() const;
        private:
          Values values_;
        };

        static SparseRowMatrixHandle make(size_type rows, size_type cols, const Values& values);
        static SparseRowMatrixHandle make(size_type rows, size_type cols, const SymmetricValues& values);
        static SparseRowMatrixHandle appendToSparseMatrix(size_type rows, size_type cols, const SparseRowMatrix& sparse, const Values& additionalValues);
        static SparseRowMatrixHandle appendToSparseMatrixSumming(size_type rows, size_type cols, const SparseRowMatrix& sparse, const Values& additionalValues);
      private:
        SparseRowMatrixFromMap();

        static size_type get_nnz(const Values& data);
      };

      template <typename T>
      class LegacySparseDataContainer
      {
      public:
        typedef boost::shared_array<index_type> Rows;
        typedef boost::shared_array<index_type> Columns;
        typedef boost::shared_array<T> Storage;

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
