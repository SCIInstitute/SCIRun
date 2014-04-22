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
   
   author: Moritz Dannhauer
   last change: 04/21/14
*/

#include <gtest/gtest.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;

DenseColumnMatrixHandle densecolumnmatrix()  
{
    DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));

    (*m)(0) = 1;
    (*m)(1) = 2;
    (*m)(2) = 3;

    return m;
}

DenseMatrixHandle densematrix()  
{
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));

    (*m)(0,0) = 1;
    (*m)(1,0) = 2;
    (*m)(2,0) = 3;

    return m;
}


TEST(MatrixTypeConversionTests, dense2column)
{
   DenseMatrixHandle from(densematrix());
    
   DenseColumnMatrixHandle result =  matrix_convert::to_column(from);
   
   EXPECT_EQ(from->ncols(), result->ncols());
   EXPECT_EQ(from->nrows(), result->nrows());
   
   for (int i = 0; i < result->nrows(); i++)
    for (int j = 0; j < result->ncols(); j++)
        EXPECT_EQ((*result)(i, j),(*from)(i, j));
}

//TEST(MatrixTypeConversionTest, column2dense)
//{
 /*  ColumnMatrix from = MAKE_COLUMN_MATRIX((1) (2) (3));
   
   DenseMatrix expected_result = MAKE_DENSE_MATRIX(
    (1)
    (2)
    (3));*/
     
    
//}



