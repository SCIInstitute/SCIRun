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

   Author:              Moritz Dannhauer
   last Modificaton:    April 13 2014
   TODO:                check Sparse inputs, Matrix inputs broken
*/


#include <gtest/gtest.h>

#include <Core/Algorithms/Math/SelectSubMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3, 4));
    for (int i = 0; i < m->rows(); i++)
      for (int j = 0; j < m->cols(); j++)
        (*m)(i, j) = 3.0 * i + j - 5;
    return m;
  }

  DenseMatrixHandle matrix2()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(5, 5));
    for (int i = 0; i < m->rows(); i++)
      for (int j = 0; j < m->cols(); j++)
        (*m)(i, j) = -3.0 * i + j + 5;
    return m;
  }

  DenseMatrixHandle matrix3()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(8, 3));
    for (int i = 0; i < m->rows(); i++)
      for (int j = 0; j < m->cols(); j++)
        (*m)(i, j) = 30.0 * i + j + 5;
    return m;
  }


  DenseMatrixHandle index_matrix_outside1()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 3));

    (*m)(0, 0) = 1;
    (*m)(1, 0) = 6;
    (*m)(0, 1) = 8;
    (*m)(1, 1) = 10;

    return m;
  }


  DenseMatrixHandle index_matrix_outside2()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 3));

    (*m)(0, 0) = std::numeric_limits<double>::quiet_NaN();
    (*m)(1, 0) = -3;

    return m;
  }

  DenseMatrixHandle index_matrix_outside3()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 3));

    (*m)(0, 0) = -1;
    (*m)(1, 0) = -2;

    return m;
  }

  DenseMatrixHandle index_matrix_outside4()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 3));

    (*m)(0, 0) = 9;
    (*m)(1, 0) = 4;

    return m;
  }

  DenseMatrixHandle index_matrix1()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 3));

    (*m)(0, 0) = 1;
    (*m)(1, 0) = 2;
    (*m)(0, 1) = 4;
    (*m)(1, 1) = 6;
    (*m)(0, 2) = 0;
    (*m)(1, 2) = 0;

    return m;
  }

  DenseMatrixHandle index_matrix2()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 1));

    (*m)(0, 0) = 1;
    (*m)(1, 0) = 3;

    return m;
  }

  DenseMatrixHandle result_matrix3_index_matrix1_rows()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(6, 3));

    (*m)(0, 0) = 35;
    (*m)(0, 1) = 36;
    (*m)(0, 2) = 37;

    (*m)(1, 0) = 125;
    (*m)(1, 1) = 126;
    (*m)(1, 2) = 127;

    (*m)(2, 0) = 5;
    (*m)(2, 1) = 6;
    (*m)(2, 2) = 7;

    (*m)(3, 0) = 65;
    (*m)(3, 1) = 66;
    (*m)(3, 2) = 67;

    (*m)(4, 0) = 185;
    (*m)(4, 1) = 186;
    (*m)(4, 2) = 187;

    (*m)(5, 0) = 5;
    (*m)(5, 1) = 6;
    (*m)(5, 2) = 7;


    return m;
  }

  DenseColumnMatrixHandle densecolumnmatrix()
  {
    DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(10));

    (*m)(0) = 6;
    (*m)(1) = 8;
    (*m)(2) = 3;
    (*m)(3) = 5;
    (*m)(4) = 0;
    (*m)(5) = 2;
    (*m)(6) = -3;
    (*m)(7) = -1;
    (*m)(8) = -6;
    (*m)(9) = -4;

    return m;
  }

  DenseMatrixHandle densecolumnmatrix_expectedoutput()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2,1));

    (*m)(0) = 8;
    (*m)(1) = 5;

    return m;
  }

  DenseMatrixHandle densecolumnmatrix_expectedoutput2()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));

    (*m)(0) = 8;
    (*m)(1) = 3;
    (*m)(2) = 5;

    return m;
  }


  DenseMatrixHandle result_matrix2_index_matrix2_rows()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(5, 2));

    (*m)(0, 0) = 6;
    (*m)(0, 1) = 8;
    (*m)(1, 0) = 3;
    (*m)(1, 1) = 5;
    (*m)(2, 0) = 0;
    (*m)(2, 1) = 2;
    (*m)(3, 0) = -3;
    (*m)(3, 1) = -1;
    (*m)(4, 0) = -6;
    (*m)(4, 1) = -4;

    return m;
  }

  DenseMatrixHandle index_matrix3()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 1));

    (*m)(0, 0) = 0;
    (*m)(1, 0) = 1;

    return m;
  }

  DenseMatrixHandle index_matrix4()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1, 2));

    (*m)(0, 0) = 1;
    (*m)(0, 1) = 0;

    return m;
  }

  const DenseMatrix Zero(DenseMatrix::Zero(3,3));

  DenseMatrixHandle result_matrix1_index_matrix34_rows()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2, 2));
    (*m)(0, 0) = -2;
    (*m)(0, 1) = -1;
    (*m)(1, 0) = -5;
    (*m)(1, 1) = -4;

     return m;
  }

  DenseMatrixHandle result_Gui_Submatrix_rowsandcolumns()
  {
   DenseMatrixHandle m(boost::make_shared<DenseMatrix>(5, 2));

   (*m)(0, 0) = 36;
   (*m)(0, 1) = 37;
   (*m)(1, 0) = 66;
   (*m)(1, 1) = 67;
   (*m)(2, 0) = 96;
   (*m)(2, 1) = 97;
   (*m)(3, 0) = 126;
   (*m)(3, 1) = 127;
   (*m)(4, 0) = 156;
   (*m)(4, 1) = 157;

   return m;
  }

 DenseMatrixHandle result_Gui_Submatrix_columnsonly()
 {
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(8, 2));

   (*m)(0, 0) = 6;
   (*m)(0, 1) = 7;
   (*m)(1, 0) = 36;
   (*m)(1, 1) = 37;
   (*m)(2, 0) = 66;
   (*m)(2, 1) = 67;
   (*m)(3, 0) = 96;
   (*m)(3, 1) = 97;
   (*m)(4, 0) = 126;
   (*m)(4, 1) = 127;
   (*m)(5, 0) = 156;
   (*m)(5, 1) = 157;
   (*m)(6, 0) = 186;
   (*m)(6, 1) = 187;
   (*m)(7, 0) = 216;
   (*m)(7, 1) = 217;

   return m;

 }


 DenseMatrixHandle result_Gui_Submatrix_rowsonly()
 {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(7, 3));

   (*m)(0, 0) = 5;
   (*m)(0, 1) = 6;
   (*m)(0, 2) = 7;
   (*m)(1, 0) = 35;
   (*m)(1, 1) = 36;
   (*m)(1, 2) = 37;
   (*m)(2, 0) = 65;
   (*m)(2, 1) = 66;
   (*m)(2, 2) = 67;
   (*m)(3, 0) = 95;
   (*m)(3, 1) = 96;
   (*m)(3, 2) = 97;
   (*m)(4, 0) = 125;
   (*m)(4, 1) = 126;
   (*m)(4, 2) = 127;
   (*m)(5, 0) = 155;
   (*m)(5, 1) = 156;
   (*m)(5, 2) = 157;
   (*m)(6, 0) = 185;
   (*m)(6, 1) = 186;
   (*m)(6, 2) = 187;

   return m;
 }


 DenseMatrixHandle Gui_Submatrix_rowsandcolumns_passinputthrough()
 {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(8, 3));

   (*m)(0, 0) = 5;
   (*m)(0, 1) = 6;
   (*m)(0, 2) = 7;
   (*m)(1, 0) = 35;
   (*m)(1, 1) = 36;
   (*m)(1, 2) = 37;
   (*m)(2, 0) = 65;
   (*m)(2, 1) = 66;
   (*m)(2, 2) = 67;
   (*m)(3, 0) = 95;
   (*m)(3, 1) = 96;
   (*m)(3, 2) = 97;
   (*m)(4, 0) = 125;
   (*m)(4, 1) = 126;
   (*m)(4, 2) = 127;
   (*m)(5, 0) = 155;
   (*m)(5, 1) = 156;
   (*m)(5, 2) = 157;
   (*m)(6, 0) = 185;
   (*m)(6, 1) = 186;
   (*m)(6, 2) = 187;
   (*m)(7, 0) = 215;
   (*m)(7, 1) = 216;
   (*m)(7, 2) = 217;

   return m;
 }



}

TEST(SelectSubMatrixTests, BoundaryChecksGUI)
{
  SelectSubMatrixAlgorithm algo;

  DenseMatrixHandle m1(matrix1());
  DenseMatrixHandle m2(matrix2());
  DenseMatrixHandle m3(matrix3());

  int col_from=0, col_to=10;

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), true);
  algo.set(SelectSubMatrixAlgorithm::columnStartSpinBox(), col_from);
  algo.set(SelectSubMatrixAlgorithm::columnEndSpinBox(), col_to);
  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),DenseMatrixHandle()));

  if (result1)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (GUI input, row only)." << std::endl;
  }


  int row_from=0, row_to=10;

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);
  algo.set(SelectSubMatrixAlgorithm::rowStartSpinBox(), row_from);
  algo.set(SelectSubMatrixAlgorithm::rowEndSpinBox(), row_to);

  DenseMatrixHandle result2 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),DenseMatrixHandle()));

  if (result2)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (GUI input, column only)." << std::endl;
  }


  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), true);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);
  algo.set(SelectSubMatrixAlgorithm::columnStartSpinBox(), col_from);
  algo.set(SelectSubMatrixAlgorithm::columnEndSpinBox(), col_to);
  algo.set(SelectSubMatrixAlgorithm::rowStartSpinBox(), row_from);
  algo.set(SelectSubMatrixAlgorithm::rowEndSpinBox(), row_to);

  DenseMatrixHandle result3 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),DenseMatrixHandle()));

  if (result3)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (GUI input, column and rows)." << std::endl;
  }

}

TEST(SelectSubMatrixTests, BoundaryChecksInputMatrices)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_rows(index_matrix1());

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,index_rows,DenseMatrixHandle()));

  if (!result1)
  {
    std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (Matrix input, rows only)." << std::endl;
  }

  EXPECT_EQ(result1->nrows(), index_rows->nrows()*index_rows->ncols());
  EXPECT_EQ(result1->ncols(), m1->ncols());

}

TEST(SelectSubMatrixTests, IndexMatricesOutOfBound_1)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_rows(index_matrix_outside1());
  DenseMatrixHandle result1;
  try
  {
    result1 =  castMatrix::toDense(algo.run(m1,index_rows,DenseMatrixHandle()));
  }
  catch (...)
  {

  }

  if (result1)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_1 - first)." << std::endl;
  }

  DenseMatrixHandle index_col(index_matrix_outside1());

  DenseMatrixHandle result2;
  try
  {
    result2 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),index_col));
  }
  catch (...)
  {

  }

  if (result2)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_1 - second)." << std::endl;
  }
}

TEST(SelectSubMatrixTests, IndexMatricesOutOfBound_2)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_rows(index_matrix_outside2());
  DenseMatrixHandle result1;

  try
  {
    result1 =  castMatrix::toDense(algo.run(m1,index_rows,DenseMatrixHandle()));
  }
  catch (...)
  {

  }

  if (result1)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_2 - first)." << std::endl;
  }

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_cols(index_matrix_outside2());
  DenseMatrixHandle result2;

  try
  {
    result2 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),index_cols));
  }
  catch (...)
  {

  }

  if (result1)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_2 - second)." << std::endl;
  }
}

TEST(SelectSubMatrixTests, IndexMatricesOutOfBound_3)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_rows(index_matrix_outside3());
  DenseMatrixHandle result1;

  try
  {
    result1 =  castMatrix::toDense(algo.run(m1,index_rows,DenseMatrixHandle()));
  }
  catch (...)
  {

  }

  if (result1)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_3 - first)." << std::endl;
  }

  DenseMatrixHandle index_cols(index_matrix_outside3());
  DenseMatrixHandle result2;

  try
  {
    result2 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),index_cols));
  }
  catch (...)
  {

  }

  if (result2)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_3 - first)." << std::endl;
  }

}

TEST(SelectSubMatrixTests, IndexMatricesOutOfBound_4)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_rows(index_matrix_outside4());
  DenseMatrixHandle result1;

  try
  {
    result1 =  castMatrix::toDense(algo.run(m1,index_rows,DenseMatrixHandle()));
  }
  catch (...)
  {

  }

  if (result1)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_4 - first)." << std::endl;
  }

  DenseMatrixHandle index_cols(index_matrix_outside4());
  DenseMatrixHandle result2;

  try
  {
    result2 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),index_cols));
  }
  catch (...)
  {

  }

  if (result2)
  {
   std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (IndexMatricesOutOfBound_4 - second)." << std::endl;
  }

}

TEST(SelectSubMatrixTests, Gui_Submatrix_rowsandcolumns_passinputthrough)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), true);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);

  int row_from=0,row_to=7,col_from=0,col_to=2;
  algo.set(SelectSubMatrixAlgorithm::columnStartSpinBox(), col_from);
  algo.set(SelectSubMatrixAlgorithm::columnEndSpinBox(), col_to);
  algo.set(SelectSubMatrixAlgorithm::rowStartSpinBox(), row_from);
  algo.set(SelectSubMatrixAlgorithm::rowEndSpinBox(), row_to);

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),DenseMatrixHandle()));

  DenseMatrixHandle expected_result(Gui_Submatrix_rowsandcolumns_passinputthrough());

  EXPECT_EQ(m1->nrows(), result1->nrows());
  EXPECT_EQ(m1->ncols(), result1->ncols());

  for (int i = 0; i < result1->nrows(); i++)
    for (int j = 0; j < result1->ncols(); j++)
        EXPECT_EQ((*expected_result)(i, j),(*result1)(i, j));

}


TEST(SelectSubMatrixTests, Gui_Submatrix_rowsonly)
{
  SelectSubMatrixAlgorithm algo;

  DenseMatrixHandle m1(matrix3());
  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);
  int row_from=0,row_to=6;

  algo.set(SelectSubMatrixAlgorithm::rowStartSpinBox(), row_from);
  algo.set(SelectSubMatrixAlgorithm::rowEndSpinBox(), row_to);


  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),DenseMatrixHandle()));

  DenseMatrixHandle expected_result(result_Gui_Submatrix_rowsonly());

  if (!result1)
  {
    std::cout << "ERROR (GUI Columns only): output is not NULL." << std::endl;
  }

  EXPECT_EQ(m1->ncols(), result1->ncols());
  EXPECT_EQ(row_to-row_from+1, result1->nrows());

  for (int i = 0; i < result1->nrows(); i++)
   for (int j = 0; j < result1->ncols(); j++)
	EXPECT_EQ((*expected_result)(i,j), (*result1)(i,j));
}

TEST(SelectSubMatrixTests, Gui_Submatrix_columnsonly)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), true);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);
  int col_from=1,col_to=2;

  algo.set(SelectSubMatrixAlgorithm::columnStartSpinBox(), col_from);
  algo.set(SelectSubMatrixAlgorithm::columnEndSpinBox(), col_to);

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),DenseMatrixHandle()));

  DenseMatrixHandle expected_result(result_Gui_Submatrix_columnsonly());

  if (!result1)
  {
    std::cout << "ERROR (GUI Columns only): output is not NULL." << std::endl;
  }

  EXPECT_EQ(m1->nrows(), result1->nrows());
  EXPECT_EQ(col_to-col_from+1, result1->ncols());

  for (int i = 0; i < result1->nrows(); i++)
   for (int j = 0; j < result1->ncols(); j++)
	EXPECT_EQ((*expected_result)(i,j), (*result1)(i,j));

}

TEST(SelectSubMatrixTests, Gui_Submatrix_rowsandcolumns)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), true);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);

  int row_from=1,row_to=5,col_from=1,col_to=2;
  algo.set(SelectSubMatrixAlgorithm::columnStartSpinBox(), col_from);
  algo.set(SelectSubMatrixAlgorithm::columnEndSpinBox(), col_to);
  algo.set(SelectSubMatrixAlgorithm::rowStartSpinBox(), row_from);
  algo.set(SelectSubMatrixAlgorithm::rowEndSpinBox(), row_to);

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),DenseMatrixHandle()));

  DenseMatrixHandle expected_result(result_Gui_Submatrix_rowsandcolumns());

  EXPECT_EQ(row_to-row_from+1, result1->nrows());
  EXPECT_EQ(col_to-col_from+1, result1->ncols());

  for (int i = 0; i < result1->rows(); i++)
   for (int j = 0; j < result1->cols(); j++)
	EXPECT_EQ((*expected_result)(i, j),(*result1)(i, j));

}

TEST(SelectSubMatrixTests, Pipe_InputMatrix_through)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());
  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::columnStartSpinBox(), -100);
  algo.set(SelectSubMatrixAlgorithm::columnEndSpinBox(), 1000);
  algo.set(SelectSubMatrixAlgorithm::rowStartSpinBox(), -1000);
  algo.set(SelectSubMatrixAlgorithm::rowEndSpinBox(), 10000);

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,0,0));

  for (int i = 0; i < result1->rows(); i++)
   for (int j = 0; j < result1->cols(); j++)
	EXPECT_EQ((*m1)(i, j),(*result1)(i, j));
}

TEST(SelectSubMatrixTests, InputMatrix_empty)
{
  SelectSubMatrixAlgorithm algo;

  DenseMatrixHandle m2;
  DenseMatrixHandle result2 =  castMatrix::toDense(algo.run(m2,0,0));

  if (result2)
  {
    std::cout << "ERROR (indices NULL, DenseMatrixHandle empty): output is not NULL." << std::endl;
  }

  DenseMatrixHandle result3 =  castMatrix::toDense(algo.run(0,0,0));

  if (result3)
  {
    std::cout << "ERROR (indices NULL, DenseMatrixHandle NULL): output is not NULL." << std::endl;
  }
}

TEST(SelectSubMatrixTests, InputMatrix_rowsonly)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix3());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_rows(index_matrix1());

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,index_rows,DenseMatrixHandle()));

  if (!result1)
  {
    std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (Matrix input, rows only)." << std::endl;
  }

  EXPECT_EQ(result1->nrows(), index_rows->nrows()*index_rows->ncols());
  EXPECT_EQ(result1->ncols(), m1->ncols());

  DenseMatrixHandle expected_result(result_matrix3_index_matrix1_rows());

  for (int i = 0; i < result1->rows(); i++)
   for (int j = 0; j < result1->cols(); j++)
	EXPECT_EQ((*expected_result)(i, j),(*result1)(i, j));
}


TEST(SelectSubMatrixTests, InputMatrix_columnsonly)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix2());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_cols(index_matrix2());

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),index_cols));

  if (!result1)
  {
    std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (Matrix input, rows only)." << std::endl;
  }

  EXPECT_EQ(result1->ncols(), index_cols->nrows()*index_cols->ncols());
  EXPECT_EQ(result1->nrows(), m1->nrows());

  DenseMatrixHandle expected_result(result_matrix2_index_matrix2_rows());

  for (int i = 0; i < result1->rows(); i++)
      for (int j = 0; j < result1->cols(); j++)
	EXPECT_EQ((*expected_result)(i, j),(*result1)(i, j));
}

TEST(SelectSubMatrixTests, InputMatrix_rowsandcolumns)
{
  SelectSubMatrixAlgorithm algo;
  DenseMatrixHandle m1(matrix1());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), false);

  DenseMatrixHandle index_rows(index_matrix4());
  DenseMatrixHandle index_cols(index_matrix3());

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,index_rows,index_cols));

  if (!result1)
  {
    std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (Matrix input, rows only)." << std::endl;
  }

  EXPECT_EQ(result1->nrows()*result1->ncols(), index_rows->nrows()*index_rows->ncols()*index_cols->nrows()*index_cols->ncols());
  DenseMatrixHandle expected_result(result_matrix1_index_matrix34_rows());

    for (int i = 0; i < result1->rows(); i++)
      for (int j = 0; j < result1->cols(); j++)
	EXPECT_EQ((*expected_result)(i, j),(*result1)(i, j));
}

TEST(SelectSubMatrixTests, NOGUI_DenseColumnMatrixCols)
{
  SelectSubMatrixAlgorithm algo;
  DenseColumnMatrixHandle m1(densecolumnmatrix());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);

  DenseMatrixHandle index_cols(index_matrix2());

  DenseMatrixHandle result1;

  try
  {
    result1 =  castMatrix::toDense(algo.run(m1,DenseMatrixHandle(),index_cols));
  }
  catch (...)
  {

  }

  if (result1)
  {
    std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (Gui input, cols only)." << std::endl;
  }
}


TEST(SelectSubMatrixTests, NOGUI_DenseColumnMatrixRows)
{
  SelectSubMatrixAlgorithm algo;
  DenseColumnMatrixHandle m1(densecolumnmatrix());

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);

  DenseMatrixHandle index_cols(index_matrix2());

  DenseMatrixHandle result1;

  try
  {
    result1 =  castMatrix::toDense(algo.run(m1,index_cols,DenseMatrixHandle()));
  }
  catch (...)
  {

  }

  if (!result1)
  {
    std::cout << "ERROR: This error message should not occur - SelectSubMatrix might seriously be broken (Gui input, cols only)." << std::endl;
  }

  EXPECT_EQ(result1->ncols(), index_cols->ncols());
  EXPECT_EQ(result1->nrows(), index_cols->nrows());

  DenseMatrixHandle expected_result(densecolumnmatrix_expectedoutput());

  for (int i = 0; i < result1->rows(); i++)
      for (int j = 0; j < result1->cols(); j++)
	EXPECT_EQ((*expected_result)(i, j),(*result1)(i, j));

}

TEST(SelectSubMatrixTests, GUI_DenseColumnMatrixRows)
{
  SelectSubMatrixAlgorithm algo;
  DenseColumnMatrixHandle m1(densecolumnmatrix());

  int from=1, to=3;

  algo.set(SelectSubMatrixAlgorithm::columnCheckBox(), false);
  algo.set(SelectSubMatrixAlgorithm::rowCheckBox(), true);

  algo.set(SelectSubMatrixAlgorithm::rowStartSpinBox(), from);
  algo.set(SelectSubMatrixAlgorithm::rowEndSpinBox(), to);

  DenseMatrixHandle result1 =  castMatrix::toDense(algo.run(m1,0,0));

  DenseMatrixHandle expected_result(densecolumnmatrix_expectedoutput2());

  EXPECT_EQ(result1->ncols(), m1->ncols());
  EXPECT_EQ(to-from+1, expected_result->nrows());

  for (int i = 0; i < result1->rows(); i++)
   for (int j = 0; j < result1->cols(); j++)
     EXPECT_EQ((*expected_result)(i, j),(*result1)(i, j));

}
