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


//ComputePCA Algorithm: Computes Principal Component Analysis.

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/ComputePCA.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Eigen/SVD>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

//Let's do some math.
//Algorithm:
void ComputePCAAlgo::run(MatrixHandle input, DenseMatrixHandle& LeftPrinMat, DenseMatrixHandle& PrinVals, DenseMatrixHandle& RightPrinMat) const{

    //Throws an error if one or both of the input matrix dimensions is zero.
    if (input->nrows() == 0 || input->ncols() == 0){

        THROW_ALGORITHM_INPUT_ERROR("Input has a zero dimension.");
    }

    //Input matrix: nxm
    if (matrixIs::dense(input))
    {
        //First, we have to center the data.
        auto denseInputCentered = centerData(input);

        //After the data is centered, then we compute SVD on the centered matrix.
        //Centered Matrix = U*S*Vt, Vt = V transpose
        Eigen::JacobiSVD<DenseMatrix::EigenBase> svd_mat(denseInputCentered, Eigen::ComputeFullU | Eigen::ComputeFullV);

        //U: Left principal matrix, nxn, orthogonal
        LeftPrinMat = boost::make_shared<DenseMatrix>(svd_mat.matrixU());

        //S: Principal values nxm, diagonal
        PrinVals = boost::make_shared<DenseMatrix>(svd_mat.singularValues());

        //V: Right singular mxm, orthognol
        RightPrinMat = boost::make_shared<DenseMatrix>(svd_mat.matrixV());
    }
    else
    {
        //Throw an error if the matrix is not dense.
        //Sparse matrices not supported at this time.
        THROW_ALGORITHM_INPUT_ERROR("ComputePCA works for dense matrix input only.");
    }
}

//Centers input matrix.
DenseMatrix ComputePCAAlgo::centerData(MatrixHandle input_matrix)
{
    //Casts the matrix as dense.
    auto denseInput = castMatrix::toDense(input_matrix);

    //Counts the number of rows in the input matrix.
    auto rows = denseInput->rows();

    //Calulates the centering matrix (C).
    // C = Identity(nxn) - 1/n * matrix of ones(nxn)
    auto centerMatrix = Eigen::MatrixXd::Identity(rows,rows) - (1.0/rows)*Eigen::MatrixXd::Constant(rows,rows,1);

    //Multiplying the input matrix by the centering matrix.
    auto denseInputCentered = centerMatrix * *denseInput;

    return denseInputCentered;
}

//Run the algorithm.
AlgorithmOutput ComputePCAAlgo::run(const AlgorithmInput& input) const
{
    auto input_matrix = input.get<Matrix>(Variables::InputMatrix);

    DenseMatrixHandle LeftPrinMat;
    DenseMatrixHandle PrinVals;
    DenseMatrixHandle RightPrinMat;

    run(input_matrix, LeftPrinMat, PrinVals, RightPrinMat);

    AlgorithmOutput output;

    output[LeftPrincipalMatrix] = LeftPrinMat;
    output[PrincipalValues] = PrinVals;
    output[RightPrincipalMatrix] = RightPrinMat;

    return output;
}

//Outputs:
AlgorithmOutputName ComputePCAAlgo::LeftPrincipalMatrix("LeftPrincipalMatrix");
AlgorithmOutputName ComputePCAAlgo::PrincipalValues("PrincipalValues");
AlgorithmOutputName ComputePCAAlgo::RightPrincipalMatrix("RightPrincipalMatrix");
