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
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;

DenseColumnMatrixHandle CreateColumnMatrix()  
{
    DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));

    (*m)(0) = 1;
    (*m)(1) = 2;
    (*m)(2) = 3;

    return m;
}

SparseRowMatrixHandle CreateSparseMatrixWithOneColumn()  
{ 
    SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,1));
    m->insert(0,0) = 1;
    m->insert(1,0) = 2;
    m->insert(2,0) = 3;
    m->makeCompressed();
    return m;
}

DenseColumnMatrixHandle CreateColumnMatrix_2()  
{
    DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));

    (*m)(0) = 1;
    (*m)(1) = 0;
    (*m)(2) = 0;
    return m;
}

DenseMatrixHandle CreateDenseMatrix_2()  
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

SparseRowMatrixHandle CreateSparseMatrix()  
{ 
    SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
    m->insert(0,0) = 1;
    m->insert(1,1) = 2;
    m->insert(2,2) = 3;
    m->makeCompressed();
    return m;
}

DenseMatrixHandle CreateDenseMatrix()  
{
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));

    (*m)(0,0) = 1;
    (*m)(1,0) = 2;
    (*m)(2,0) = 3;

    return m;
}

int WhatMatrixTypeIsIt(MatrixHandle o)
{
  if(matrix_is::dense(o))
  {
    return 1;
  }
  else if (matrix_is::column(o))
  {
    return 2;
  }
  else if (matrix_is::sparse(o))
  {
    return 3;
  }
  else
  {
    return -1;
  }
}

TEST(ConvertMatrixTests, EmptyInput)
{
  ConvertMatrixTypeAlgorithm algo;
  EXPECT_THROW(algo.run(0), AlgorithmInputException);
  EXPECT_THROW(algo.run(MatrixHandle()), AlgorithmInputException);
}

TEST(ConvertMatrixTests, PassInputThrough)
{  
  
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);
  
  MatrixHandle input1(CreateDenseMatrix());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  if (!output_matrix1)
  {
   FAIL() << "ERROR: DenseMatrix input for ConvertMatrixTypeAlgorithm does not work." << std::endl;
  }  
  
  auto out1 = matrix_cast::as_dense(output_matrix1);
  auto in1 = matrix_cast::as_dense(input1);
  
  EXPECT_EQ(in1->nrows(), out1->nrows());
  EXPECT_EQ(in1->ncols(), out1->ncols());
    
  for (int i = 0; i < output_matrix1->nrows(); i++)
   for (int j = 0; j < output_matrix1->ncols(); j++)
        EXPECT_EQ((*in1)(i, j),(*out1)(i, j));  
  
  MatrixHandle input2(CreateColumnMatrix());
  
  MatrixHandle output_matrix2 = algo.run(input2);  
  if (!output_matrix2)
  {
   FAIL() << "ERROR: DenseColumnMatrix input for ConvertMatrixTypeAlgorithm does not work." << std::endl;
  }
 
  auto out2 = matrix_cast::as_column(output_matrix2);
  auto in2 = matrix_cast::as_column(input2); 
  
  EXPECT_EQ(in2->nrows(), out2->nrows());
  EXPECT_EQ(in2->ncols(), out2->ncols());
    
  for (int i = 0; i < output_matrix2->nrows(); i++)
   for (int j = 0; j < output_matrix2->ncols(); j++)
        EXPECT_EQ((*in2)(i, j),(*out2)(i, j)); 

 }

TEST(ConvertMatrixTests, DenseToColumnMatrix)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);

  MatrixHandle input1(CreateDenseMatrix());
  MatrixHandle output_matrix1 = algo.run(input1);
      
  auto expected_result = CreateColumnMatrix();
  auto out1 = matrix_cast::as_column(output_matrix1);
  
  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
 for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ((*expected_result)(i, j),(*out1)(i, j));  
}

TEST(ConvertMatrixTests, DenseToSparseMatrix)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), true);

  MatrixHandle input1(CreateDenseMatrix_2());
  MatrixHandle output_matrix1 = algo.run(input1); 
  
  DenseMatrixHandle input=matrix_cast::as_dense(input1);
  auto output = matrix_cast::as_sparse(output_matrix1);
  
  EXPECT_EQ(input->nrows(), output->nrows());
  EXPECT_EQ(input->ncols(), output->ncols());   
 
  int count_dense=0;
  for (int i = 0; i < input->nrows(); i++)
   for (int j = 0; j < input->ncols(); j++)
        if ((*input)(i,j)!=0) count_dense++;
 
  EXPECT_EQ(count_dense, output->nonZeros());
 
  for (index_type row = 0; row < output->outerSize(); row++)
    {
      for (SparseRowMatrix::InnerIterator it(*output,row); it; ++it)
        EXPECT_EQ(it.value(),(*input)(row, it.index()));   
    }
  
}

TEST(ConvertMatrixTests, ColumnToDenseMatrix)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);
  
  MatrixHandle input1(CreateColumnMatrix());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  auto expected_result = CreateDenseMatrix();
  auto out1 = matrix_cast::as_dense(output_matrix1);   
  
  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
 for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ((*expected_result)(i, j),(*out1)(i, j));
}

TEST(ConvertMatrixTests, ColumnToSparseMatrix)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), true);
  
  MatrixHandle input1(CreateColumnMatrix());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  auto expected_result = CreateSparseMatrixWithOneColumn();
  auto out1 = matrix_cast::as_sparse(output_matrix1);   

  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
  for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ(expected_result->coeff(i, j),out1->coeff(i, j)); 
}

TEST(ConvertMatrixTests, SparseToColumnMatrix)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);

  MatrixHandle input1(CreateSparseMatrixWithOneColumn());
  MatrixHandle output_matrix1 = algo.run(input1);
  
  auto expected_result = CreateColumnMatrix();
  auto out1 = matrix_cast::as_column(output_matrix1);  
  
  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
  
  for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ(expected_result->coeff(i, j),out1->coeff(i, j)); 
  
}

TEST(ConvertMatrixTests, SparseToDenseMatrix)
{ 
  ConvertMatrixTypeAlgorithm algo;
  
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);

  MatrixHandle input1(CreateSparseMatrix());
  MatrixHandle output_matrix1 = algo.run(input1);

  auto expected_result = CreateDenseMatrix_2();
  auto out1 = matrix_cast::as_dense(output_matrix1);  

  EXPECT_EQ(expected_result->nrows(), out1->nrows());
  EXPECT_EQ(expected_result->ncols(), out1->ncols()); 
 
  for (int i = 0; i < out1->nrows(); i++)
   for (int j = 0; j < out1->ncols(); j++)
        EXPECT_EQ(expected_result->coeff(i, j),out1->coeff(i, j)); 
 
}

// The priority matrix type is as follows: PassThrough, Column, Sparse, Dense

TEST(ConvertMatrixTests, AllRadiosSetToTrueInputDenseMatrix)
{
  ConvertMatrixTypeAlgorithm algo;
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), true);
  MatrixHandle input(CreateDenseMatrix());
  MatrixHandle output = algo.run(input);
  EXPECT_EQ(1,WhatMatrixTypeIsIt(output));
}

TEST(ConvertMatrixTests, AllRadiosSetToTrueInputSparseMatrix)
{
  ConvertMatrixTypeAlgorithm algo;
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), true);
  MatrixHandle input(CreateSparseMatrix());
  MatrixHandle output = algo.run(input);
  EXPECT_EQ(3,WhatMatrixTypeIsIt(output));
}

TEST(ConvertMatrixTests, AllRadiosSetToTrueInputColumnMatrix)
{
  ConvertMatrixTypeAlgorithm algo;
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), true);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), true);
  MatrixHandle input(CreateColumnMatrix());
  MatrixHandle output = algo.run(input);
  EXPECT_EQ(2,WhatMatrixTypeIsIt(output));
}

TEST(ConvertMatrixTests, AllRadiosSetToFalseInputSparseMatrix)
{
  ConvertMatrixTypeAlgorithm algo;
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);
  MatrixHandle input(CreateSparseMatrix());
  MatrixHandle output = algo.run(input);
  EXPECT_EQ(3,WhatMatrixTypeIsIt(output));
}

TEST(ConvertMatrixTests, AllRadiosSetToFalseInputDenseMatrix)
{
  ConvertMatrixTypeAlgorithm algo;
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);
  MatrixHandle input(CreateDenseMatrix());
  MatrixHandle output = algo.run(input);
  EXPECT_EQ(1,WhatMatrixTypeIsIt(output));
}

TEST(ConvertMatrixTests, AllRadiosSetToFalseInputColumnMatrix)
{
  ConvertMatrixTypeAlgorithm algo;
  algo.set(ConvertMatrixTypeAlgorithm::PassThrough(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), false);
  algo.set(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), false);
  MatrixHandle input(CreateColumnMatrix());
  MatrixHandle output = algo.run(input);
  EXPECT_EQ(2,WhatMatrixTypeIsIt(output));
}

