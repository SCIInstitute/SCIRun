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

void ComputePCAAlgo::run(MatrixHandle input, DenseMatrixHandle& LeftPrinMat, DenseMatrixHandle& PrinVals, DenseMatrixHandle& RightPrinMat) const
{
    if (matrix_is::dense(input))
    {
        auto denseInput = matrix_cast::as_dense(input);
        
        auto rows = denseInput->rows();
        
        auto centerMatrix = Eigen::MatrixXd::Identity(rows,rows) - (1/rows)*Eigen::MatrixXd::Constant(rows,rows,1);
        
        auto denseInputCentered = centerMatrix * *denseInput;
        
        Eigen::JacobiSVD<DenseMatrix::EigenBase> svd_mat(denseInputCentered, Eigen::ComputeFullU | Eigen::ComputeFullV);
        
        LeftPrinMat = boost::make_shared<DenseMatrix>(svd_mat.matrixU());
        
        PrinVals = boost::make_shared<DenseMatrix>(svd_mat.singularValues());
        
        RightPrinMat = boost::make_shared<DenseMatrix>(svd_mat.matrixV());
    }
    else
    {
        THROW_ALGORITHM_INPUT_ERROR("ComputePCA works for dense matrix input only.");
    }
}


AlgorithmOutput ComputePCAAlgo::run_generic(const AlgorithmInput& input) const
{
    auto input_matrix = input.get<Matrix>(Variables::InputMatrix);
    
    DenseMatrixHandle LeftPrinMat;
    DenseMatrixHandle RightPrinMat;
    DenseMatrixHandle PrinVals;
    
    run(input_matrix, LeftPrinMat, PrinVals, RightPrinMat);
    
    AlgorithmOutput output;
    
    output[LeftPrincipalMatrix] = LeftPrinMat;
    output[PrincipalValues] = PrinVals;
    output[RightPrincipalMatrix] = RightPrinMat;
    
    return output;
}

AlgorithmOutputName ComputePCAAlgo::LeftPrincipalMatrix("LeftPrincipalMatrix");
AlgorithmOutputName ComputePCAAlgo::PrincipalValues("PrincipalValues");
AlgorithmOutputName ComputePCAAlgo::RightPrincipalMatrix("RightPrincipalMatrix");
