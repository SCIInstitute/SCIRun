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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/ComputeSVD.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Eigen/SVD>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

void ComputeSVDAlgo::run(MatrixHandle input, DenseMatrixHandle& LeftSingMat, DenseMatrixHandle& SingVals, DenseMatrixHandle& RightSingMat) const
{
  if (input->nrows() == 0 || input->ncols() == 0){

    THROW_ALGORITHM_INPUT_ERROR("Input has a zero dimension.");
}
  if (matrixIs::dense(input))
  {
    auto denseInput = castMatrix::toDense(input);

    Eigen::JacobiSVD<DenseMatrix::EigenBase> svd_mat(*denseInput, Eigen::ComputeFullU | Eigen::ComputeFullV);

    LeftSingMat = boost::make_shared<DenseMatrix>(svd_mat.matrixU());

    SingVals = boost::make_shared<DenseMatrix>(svd_mat.singularValues());

    RightSingMat = boost::make_shared<DenseMatrix>(svd_mat.matrixV());
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR("ComputeSVD works for dense matrix input only.");
  }
}


AlgorithmOutput ComputeSVDAlgo::run(const AlgorithmInput& input) const
{
	auto input_matrix = input.get<Matrix>(Variables::InputMatrix);

	DenseMatrixHandle LeftSingMat;
	DenseMatrixHandle RightSingMat;
	DenseMatrixHandle SingVals;

	run(input_matrix, LeftSingMat, SingVals, RightSingMat);

	AlgorithmOutput output;

	output[LeftSingularMatrix] = LeftSingMat;
	output[SingularValues] = SingVals;
	output[RightSingularMatrix] = RightSingMat;

	return output;
}

AlgorithmOutputName ComputeSVDAlgo::LeftSingularMatrix("LeftSingularMatrix");
AlgorithmOutputName ComputeSVDAlgo::SingularValues("SingularValues");
AlgorithmOutputName ComputeSVDAlgo::RightSingularMatrix("RightSingularMatrix");
