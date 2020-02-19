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


//ComputeSVD algorithm test.
#include <gtest/gtest.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Algorithms/Math/ComputeSVD.h>
#include <Eigen/SVD>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::TestUtils;

namespace
{
    //Matrix for input.
    DenseMatrixHandle inputMatrix()
    {
        //Hard coded for testing.
        int column1 [12] = {5,4,7,8,10,10,4,6,8,7,9,6};
        int column2 [12] = {7,6,9,8,5,7,9,3,4,5,5,9};

        //Puts the columns into a matrix.
        DenseMatrixHandle inputM(boost::make_shared<DenseMatrix>(12,2));
        for (int i = 0; i < inputM->rows(); i++){
            (*inputM)(i,0) = column1[i];
            (*inputM)(i,1) = column2[i];
        }
        return inputM;
    }
}

//Checks if the outputs are correct.
//U,S,V: U: Left singular matrix, S: Singular values, V: Right singular matrix.
TEST(ComputeSVDtest, checkOutputs)
{
    ComputeSVDAlgo algo;

    DenseMatrixHandle m1(inputMatrix());
    DenseMatrixHandle LeftSingularMatrix_U;
    DenseMatrixHandle SingularValues_S;
    DenseMatrixHandle RightSingularMatrix_V;

    //Runs algorithm.
    algo.run(m1,LeftSingularMatrix_U,SingularValues_S,RightSingularMatrix_V);

    //Testing if the results are null.
    ASSERT_NE(nullptr,LeftSingularMatrix_U);
    ASSERT_NE(nullptr,SingularValues_S);
    ASSERT_NE(nullptr,RightSingularMatrix_V);

    //Check the dimensions of the matrices that were created for output.

    //Rows
    ASSERT_EQ(12,LeftSingularMatrix_U->rows());
    ASSERT_EQ(2,SingularValues_S->rows());
    ASSERT_EQ(2,RightSingularMatrix_V->rows());

    //Columns
    ASSERT_EQ(12,LeftSingularMatrix_U->cols());
    ASSERT_EQ(1,SingularValues_S->cols());
    ASSERT_EQ(2,RightSingularMatrix_V->cols());

    //Eigen does not create a diagonal matrix when it computes SVD, it just has a column with the singular values, so we must put it into a diagonal matrix to be able to do some matrix multiplication later.
    DenseMatrix sDiag = Eigen::MatrixXd::Constant(12,2,0);
    sDiag.diagonal() = SingularValues_S->col(0);

    //Multiplying back together and comparing to the centered matrix. They should be equal to each other with some tolerance.
    DenseMatrix product = (*LeftSingularMatrix_U) * sDiag * (*RightSingularMatrix_V).transpose();

    auto expected = *inputMatrix();

    //Comparing each element in the matrices.
    for (int i = 0; i < product.rows(); ++i) {
        for (int j = 0; j < product.cols(); ++j)
            ASSERT_NEAR(expected(i,j), product(i,j), 1e-5);
    }

}

//Tests for input with a dimension of zero.
TEST(ComputeSVDtest, ThrowsForZeroDimensionInput)
{
    ComputeSVDAlgo algo;

    DenseMatrixHandle m1(new DenseMatrix(5, 0));
    DenseMatrixHandle m2(new DenseMatrix(0, 5));
    DenseMatrixHandle m3(new DenseMatrix(0, 0));

    DenseMatrixHandle LeftSingularMatrix_U;
    DenseMatrixHandle SingularValues_S;
    DenseMatrixHandle RightSingularMatrix_V;

    //Runs algorithm and expects an error.
    EXPECT_ANY_THROW(algo.run(m1,LeftSingularMatrix_U,SingularValues_S,RightSingularMatrix_V));
    EXPECT_ANY_THROW(algo.run(m2,LeftSingularMatrix_U,SingularValues_S,RightSingularMatrix_V));
    EXPECT_ANY_THROW(algo.run(m3,LeftSingularMatrix_U,SingularValues_S,RightSingularMatrix_V));

}
