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

#include <gtest/gtest.h>
#include <boost/test/floating_point_comparison.hpp>

#include <Testing/Util/MatrixTestUtilities.h>

#include <Core/Math/sci_lapack.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/MatrixOperations.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Core/Exceptions/LapackError.h>

using namespace boost::assign; 
using namespace SCIRun;
using namespace SCIRun::TestUtils;

TEST(InvertMatrixTest, InvertZeroException)
{
  const int dim = 3;
  DenseMatrix zero_matrix(dim, dim);
  zero_matrix.zero();
  
  EXPECT_THROW(lapackinvert(zero_matrix.get_data_pointer(), dim), LapackError);
}

TEST(InvertMatrixTest, InvertArgException)
{
  const int dim = 5;
  const int bad_dim = 4;
  DenseMatrixHandle identity(DenseMatrix::identity(dim));
  DenseMatrixHandle identityOriginal(DenseMatrix::identity(dim));
  
  EXPECT_THROW(lapackinvert(identity->get_data_pointer(), bad_dim), LapackError);
  
  EXPECT_FALSE(compare_exactly(*identity, *identityOriginal));
}

TEST(SolveLinSysWithLapackTest, SolveLinSysZeroException)
{
  const int dim = 6;
  DenseMatrix zero_matrix(dim, dim);
  zero_matrix.zero();
  ColumnMatrix rhs = MAKE_COLUMN_MATRIX((107) (60) (71) (43) (82));

  EXPECT_THROW(lapacksolvelinearsystem(zero_matrix.get_raw_2D_pointer(), dim, dim,
                                       rhs.get_data_pointer(),  dim, 1),
               LapackError);
}

TEST(InvertMatrixTest, CanInvertIdentity)
{
  const int dim = 3;
  DenseMatrixHandle identity(DenseMatrix::identity(dim));
  DenseMatrixHandle identityOriginal(DenseMatrix::identity(dim));
  
  EXPECT_NO_THROW(lapackinvert(identity->get_data_pointer(), dim));
  
  EXPECT_TRUE(compare_exactly(*identity, *identityOriginal));
}

TEST(SolveLinSysWithLapackTest, SolvingSimpleCase)
{
  DenseMatrix M = MAKE_DENSE_MATRIX(
    (2, 4, 7, 9, 8)
    (6, 9, 2, 5, 2)
    (6, 3, 5, 1, 8)
    (1, 5, 6, 1, 2)
    (1, 2, 8, 2, 9));
    
   ColumnMatrix rhs = MAKE_COLUMN_MATRIX((107) (60) (71) (43) (82));
   
   EXPECT_NO_THROW(lapacksolvelinearsystem(M.get_raw_2D_pointer(), 5, 5, rhs.get_data_pointer(),  5, 1));
   
   EXPECT_COLUMN_MATRIX_EQ_TO(rhs,
    (1.0)
    (2.0)
    (3.0)
    (4.0)
    (5.0));

}

TEST(InvertMatrixTest, CanInvertWithMemberFunction)
{
  const int rows = 3, cols = 3;

  DenseMatrix m = MAKE_DENSE_MATRIX(
    (1, 0, 1)
    (0, 2, 0)
    (0, 0, -1));

  const MatrixHandle original(m.clone());
  //std::cout << "Matrix:" << std::endl;
  //std::cout << matrix_to_string(m) << std::endl;

  EXPECT_TRUE(m.invert());
  const MatrixHandle inverseFromMethod(m.clone());
  //std::cout << "Inverse from method:" << std::endl;
  //std::cout << to_string(inverseFromMethod) << std::endl;

  EXPECT_TRUE(m.invert());
  //std::cout << "Back to original matrix:" << std::endl;
  //std::cout << matrix_to_string(m) << std::endl;

  //std::cout << "Inversion via direct call to lapack:" << std::endl;

  EXPECT_NO_THROW(lapackinvert(m.get_data_pointer(), rows));

  //std::cout << matrix_to_string(m) << std::endl;
  {
    const MatrixHandle inverseFromDirectLapack(m.clone());

    //std::cout << "Difference matrix:" << std::endl;
    MatrixHandle diff = inverseFromDirectLapack - inverseFromMethod;
    //std::cout << to_string(diff) << std::endl;

    DenseMatrixHandle zero(DenseMatrix::zero_matrix(rows, cols));

    EXPECT_TRUE(compare_exactly(*diff, *zero));
  }

  MatrixHandle id3(DenseMatrix::identity(rows));
  MatrixHandle product = inverseFromMethod * original;
  EXPECT_TRUE(compare_exactly(*id3, *product));
}

//Note: this test can be a template for further lapack function testing.
TEST(SVDTest, ExampleFromWikiPage)
{
  const int rows = 4, cols = 5;

  DenseMatrix m = MAKE_DENSE_MATRIX(
    (1, 0, 0, 0, 2)
    (0, 0, 3, 0, 0)
    (0, 0, 0, 0, 0)
    (0, 4, 0, 0, 0));

  DenseMatrix u(rows, rows);
  ColumnMatrix s(rows);
  DenseMatrix v_transpose(cols, cols);

  EXPECT_NO_THROW(lapacksvd(m.get_raw_2D_pointer(), rows, cols,
                            s.get_data_pointer(),
                            u.get_raw_2D_pointer(),
                            v_transpose.get_raw_2D_pointer()));

  EXPECT_MATRIX_EQ_TO(u, 
    (0,0,1,0)
    (0,1,0,0)
    (0,0,0,-1)
    (1,0,0,0));

  EXPECT_COLUMN_MATRIX_EQ_TO(s,
    (4.0)
    (3.0)
    (2.23606798)
    (0));

  EXPECT_MATRIX_EQ_TO(v_transpose,
    (0.0,1.0,0.0,0.0,0.0)
    (0,  0,  1,  0,  0)
    (sqrt(0.2), 0, 0, 0, sqrt(0.8))
    (0, 0, 0, 1, 0)
    (-sqrt(0.8), 0, 0, 0, sqrt(0.2)));

  MatrixHandle U(u.clone());
  MatrixHandle fullS(DenseMatrix::make_diagonal_from_column(s, rows, cols));
  MatrixHandle V_transpose(v_transpose.clone());

  EXPECT_MATRIX_EQ(m, *(U * fullS * V_transpose));
}

