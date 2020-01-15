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


//ComputePCA algorithm test.
#include <gtest/gtest.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Algorithms/Math/ComputePCA.h>
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

    //The input matrix after centering.
    DenseMatrixHandle centeredInputMatrix()
    {
        //Hard coded for testing.
        double centeredColumn1 [12] = {-2.0,-3.0,-0.0,1.0,3.0,3.0,-3.0,-1.0,1.0,-0.0,2.0,-1.0};
        double centeredColumn2 [12] = {0.583333,-0.416667,2.583333,1.583333,-1.416667,0.583333,2.583333,-3.416667,-2.416667,-1.416667,-1.416667,2.583333};

        //Puts the columns into a matrix.
        DenseMatrixHandle centeredM(boost::make_shared<DenseMatrix>(12,2));
        for (int i = 0; i < centeredM->rows(); i++){
            (*centeredM)(i,0) = centeredColumn1[i];
            (*centeredM)(i,1) = centeredColumn2[i];
        }
        return centeredM;
    }
}

//Checks if the algorithm centers the data correctly.
TEST(ComputePCAtest, CenterData)
{
    ComputePCAAlgo algo;

    DenseMatrixHandle m1(inputMatrix());

    auto centered = ComputePCAAlgo::centerData(m1);

    auto expected = *centeredInputMatrix();

    //Checking every element with a tolerance of 1e-5.
    for (int i = 0; i < centered.rows(); ++i) {
        for (int j = 0; j < centered.cols(); ++j)
            ASSERT_NEAR(expected(i,j), centered(i,j), 1e-5);
    }
}

//Checks if the outputs are correct.
//U,S,V: U: Left principal matrix, S: principal values, V: Right principal matrix.
TEST(ComputePCAtest, checkOutputs)
{
    ComputePCAAlgo algo;

    DenseMatrixHandle m1(inputMatrix());
    DenseMatrixHandle LeftPrinMat_U;
    DenseMatrixHandle PrinVals_S;
    DenseMatrixHandle RightPrinMat_V;

    //Runs algorithm.
    algo.run(m1,LeftPrinMat_U,PrinVals_S,RightPrinMat_V);

    //Testing if the results are null.
    ASSERT_NE(nullptr,LeftPrinMat_U);
    ASSERT_NE(nullptr,PrinVals_S);
    ASSERT_NE(nullptr,RightPrinMat_V);

    //Check the dimensions of the matrices that were created for output.

    //Rows
    ASSERT_EQ(12,LeftPrinMat_U->rows());
    ASSERT_EQ(2,PrinVals_S->rows());
    ASSERT_EQ(2,RightPrinMat_V->rows());

    //Columns
    ASSERT_EQ(12,LeftPrinMat_U->cols());
    ASSERT_EQ(1,PrinVals_S->cols());
    ASSERT_EQ(2,RightPrinMat_V->cols());



    //Eigen does not create a diagonal matrix when it computes SVD, it just has a column with the principal values, so we must put it into a diagonal matrix to be able to do some matrix multiplication later.
    DenseMatrix sDiag = Eigen::MatrixXd::Constant(12,2,0);
    sDiag.diagonal() = PrinVals_S->col(0);

    //Multiplying back together and comparing to the centered matrix. They should be equal to each other with some tolerance.
    DenseMatrix product = (*LeftPrinMat_U) * sDiag * (*RightPrinMat_V).transpose();

    auto expected = *centeredInputMatrix();

    //Comparing each element in the matrices.
    for (int i = 0; i < product.rows(); ++i) {
        for (int j = 0; j < product.cols(); ++j)
            ASSERT_NEAR(expected(i,j), product(i,j), 1e-5);
    }

}

//Tests for input with a dimension of zero.
TEST(ComputePCAtest, ThrowsForZeroDimensionInput)
{
    ComputePCAAlgo algo;

    DenseMatrixHandle m1(new DenseMatrix(5, 0));
    DenseMatrixHandle m2(new DenseMatrix(0, 5));
    DenseMatrixHandle m3(new DenseMatrix(0, 0));

    DenseMatrixHandle LeftPrinMat_U;
    DenseMatrixHandle PrinVals_S;
    DenseMatrixHandle RightPrinMat_V;

    //Runs algorithm and expects an error.
    EXPECT_ANY_THROW(algo.run(m1,LeftPrinMat_U,PrinVals_S,RightPrinMat_V));
    EXPECT_ANY_THROW(algo.run(m2,LeftPrinMat_U,PrinVals_S,RightPrinMat_V));
    EXPECT_ANY_THROW(algo.run(m3,LeftPrinMat_U,PrinVals_S,RightPrinMat_V));

}
