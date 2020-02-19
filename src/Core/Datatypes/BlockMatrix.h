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


#ifndef CORE_DATATYPES_BLOCK_MATRIX_H
#define CORE_DATATYPES_BLOCK_MATRIX_H

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class SCISHARE DenseBlockMatrix //: public DenseMatrix
  {
  public:
    typedef std::vector<int> RowBlockSizes;
    typedef std::vector<int> ColumnBlockSizes;
    DenseBlockMatrix(const RowBlockSizes& rbs, const ColumnBlockSizes& cbs);

    //auto blockAt(int i, int j) const -> decltype(DenseMatrix().block(0,0,0,0))
    //{
    //  return this->block(startRow(i), startCol(j), rowSize(i), colSize(j));
    //}

    auto blockRef(int i, int j) -> decltype(DenseMatrix().block(0,0,0,0))
    {
      return matrix_.block(startRow(i), startCol(j), rowSize(i), colSize(j));
    }

    DenseMatrix& matrix() { return matrix_; }
    const DenseMatrix& matrix() const { return matrix_; }
  private:
    DenseMatrix matrix_;
    std::vector<int> blockStartsRow_, blockStartsCol_;
    int startRow(int i) const;
    int startCol(int j) const;
    int rowSize(int i) const;
    int colSize(int j) const;
  };

}}}

#endif
