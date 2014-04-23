/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
   last change: 4/22/14
*/

#include <gtest/gtest.h>
#include <Core/Algorithms/Math/ConvertMatrixType.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

DenseColumnMatrixHandle densecolumnmatrix()  
{
    DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));

    (*m)(0) = 1;
    (*m)(1) = 2;
    (*m)(2) = 3;

    return m;
}

SparseRowMatrixHandle col2sparse()  
{ 
    SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,1));
    m->insert(0,0) = 1;
    m->insert(1,0) = 2;
    m->insert(2,0) = 3;
    m->makeCompressed();
    return m;
}

DenseColumnMatrixHandle densecolumnmatrix2()  
{
    DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));

    (*m)(0) = 1;
    (*m)(1) = 0;
    (*m)(2) = 0;
    return m;
}

DenseMatrixHandle densematrix2()  
{
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,3));

    (*m)(0,0) = 1;
    (*m)(0,1) = 0;
    (*m)(0,2) = 0;
    (*m)(1,0) = 0;
    (*m)(1,1) = 2;
    (*m)(1,2) = 0;
    (*m)(2,0) = 0;
    (*m)(2,1) = 0;
    (*m)(2,2) = 3;

    return m;
}

SparseRowMatrixHandle densematrix2sparse()  
{ 
    SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
    m->insert(0,0) = 1;
    m->insert(1,1) = 2;
    m->insert(2,2) = 3;
    m->makeCompressed();
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


TEST(ConvertMatrixTests, emptyinput)
{
  ConvertMatrixTypeAlgorithm algo;
  
  MatrixHandle output_matrix1;
  
  try
  {
   output_matrix1 = algo.run(0);
  }
  catch(...)
  {
  
  }
  
  if (!output_matrix1)
  {
   std::cout << "ERROR: zero input for ConvertMatrixTypeAlgorithm does not work." << std::endl;

  }
  
  MatrixHandle output_matrix2;
  try
  {
    output_matrix2 = algo.run(MatrixHandle());
  }
  catch(...)
  {
  
  }
  
  if (!output_matrix2)
  {
   std::cout << "ERROR: MatrixHandle input for ConvertMatrixTypeAlgorithm does not work." << std::endl;
  }
  
}

TEST(ConvertMatrixTests, passthrough)
{  
  
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough, true);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2ColumnMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2DenseMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2SparseRowMatrix, false);
  
  MatrixHandle input1(densematrix());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  if (!output_matrix1)
  {
   std::cout << "ERROR: DenseMatrix input for ConvertMatrixTypeAlgorithm does not work." << std::endl;
  }  
  
  auto out1 = matrix_cast::as_dense(output_matrix1);
  auto in1 = matrix_cast::as_dense(input1);
  
  EXPECT_EQ(in1->nrows(), out1->nrows());
  EXPECT_EQ(in1->ncols(), out1->ncols());
    
  for (int i = 0; i < output_matrix1->nrows(); i++)
   for (int j = 0; j < output_matrix1->ncols(); j++)
        EXPECT_EQ((*in1)(i, j),(*out1)(i, j));  
  
  MatrixHandle input2(densecolumnmatrix());
  
  MatrixHandle output_matrix2 = algo.run(input2);  
  if (!output_matrix2)
  {
   std::cout << "ERROR: DenseColumnMatrix input for ConvertMatrixTypeAlgorithm does not work." << std::endl;
  }
 
  auto out2 = matrix_cast::as_column(output_matrix2);
  auto in2 = matrix_cast::as_column(input2); 
  
  EXPECT_EQ(in2->nrows(), out2->nrows());
  EXPECT_EQ(in2->ncols(), out2->ncols());
    
  for (int i = 0; i < output_matrix2->nrows(); i++)
   for (int j = 0; j < output_matrix2->ncols(); j++)
        EXPECT_EQ((*in2)(i, j),(*out2)(i, j)); 

 }

TEST(ConvertMatrixTests, dense2col)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2ColumnMatrix, true);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2DenseMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2SparseRowMatrix, false);

  MatrixHandle input1(densematrix2());
  MatrixHandle output_matrix1 = algo.run(input1);
      
  auto expected_result = densecolumnmatrix2();
  auto out1 = matrix_cast::as_column(output_matrix1);
  
  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
 for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ((*expected_result)(i, j),(*out1)(i, j));  
}


TEST(ConvertMatrixTests, dense2sparse)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2ColumnMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2DenseMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2SparseRowMatrix, true);

  MatrixHandle input1(densematrix2());
  MatrixHandle output_matrix1 = algo.run(input1); 
  
  auto expected_result = densematrix2sparse();
  auto out1 = matrix_cast::as_sparse(output_matrix1); 
  
  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
 for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ(expected_result->coeff(i, j),out1->coeff(i, j));   
  
}

TEST(ConvertMatrixTests, col2dense)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2ColumnMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2DenseMatrix, true);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2SparseRowMatrix, false);
  
  MatrixHandle input1(densecolumnmatrix());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  auto expected_result = densematrix();
  auto out1 = matrix_cast::as_dense(output_matrix1);   
  
  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
 for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ((*expected_result)(i, j),(*out1)(i, j));
}

TEST(ConvertMatrixTests, col2sparse)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2ColumnMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2DenseMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2SparseRowMatrix, true);
  
  MatrixHandle input1(densecolumnmatrix());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  auto expected_result = col2sparse();
  auto out1 = matrix_cast::as_sparse(output_matrix1);   

  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
  for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ(expected_result->coeff(i, j),out1->coeff(i, j)); 
}

TEST(ConvertMatrixTests, sparse2col)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2ColumnMatrix, true);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2DenseMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2SparseRowMatrix, false);

  MatrixHandle input1(col2sparse());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  auto expected_result = densecolumnmatrix();
  auto out1 = matrix_cast::as_column(output_matrix1);  
  
  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
  for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ(expected_result->coeff(i, j),out1->coeff(i, j)); 
  
}

TEST(ConvertMatrixTests, sparse2dense)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2ColumnMatrix, false);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2DenseMatrix, true);
  algo.set(ConvertMatrixTypeAlgorithm::Convert2SparseRowMatrix, false);

  MatrixHandle input1(densematrix2sparse());
  MatrixHandle output_matrix1 = algo.run(input1);

  auto expected_result = densematrix2();
  auto out1 = matrix_cast::as_dense(output_matrix1);  

  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
 
  for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ(expected_result->coeff(i, j),out1->coeff(i, j)); 
 
}
