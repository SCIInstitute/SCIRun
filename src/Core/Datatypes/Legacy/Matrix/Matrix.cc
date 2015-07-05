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


///
///@file  Matrix.cc
///@brief Matrix definitions
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  July 1994
///

#include <Core/Util/Assert.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Math/MiscMath.h>
#include <Core/Datatypes/MatrixAlgorithms.h>

namespace SCIRun {

PersistentTypeID MatrixBase::type_id("MatrixBase", "PropertyManager", 0);


#define MATRIX_VERSION 3

void
MatrixBase::io(Piostream& stream)
{
  int version = stream.begin_class("Matrix", MATRIX_VERSION);
  if (version < 2) 
  {
    int tmpsym;
    stream.io(tmpsym);
  }
  if (version > 2) 
  {
    PropertyManager::io(stream);
  }
  stream.end_class();
}

void
Mult(ColumnMatrix& result, const Matrix<double>& mat, const ColumnMatrix& v)
{
  mat.mult(v, result);
}

} // End namespace SCIRun
