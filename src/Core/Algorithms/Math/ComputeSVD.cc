/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2010 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Math/ComputeSVD.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Eigen/Dense>
#include <Eigen/SVD>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

void ComputeSVDAlgo::run(MatrixHandle input_matrix, DenseMatrixHandle LeftSingMat, DenseColumnMatrixHandle SingVals, DenseMatrixHandle RightSingMat) const
{
	if(matrix_is::sparse(input_matrix))
		matrix_convert::to_dense(input_matrix);
	
	int numRows, numCols;
	Eigen::JacobiSVD<DenseMatrixGeneric<double>::EigenBase> svd_mat(input_matrix, Eigen::ComputeFullU | Eigen::ComputeFullV);
	
	DenseMatrix temp_mat = svd_mat.matrixU();
	numRows = temp_mat.nrows();
	numCols = temp_mat.ncols();
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			double value = temp_mat.get(r,c);
			LeftSingMat->put(r, c, value);
		}
	}
	
	temp_mat = svd_mat.matrixV();
	numRows = temp_mat.nrows();
	numCols = temp_mat.ncols();
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			double value = temp_mat.get(r,c);
			RightSingMat->put(r, c, value);
		}
	}
	
	DenseColumnMatrix temp_vect = svd_mat.singularValues();
	numRows = temp_vect.nrows();
	numCols = temp_vect.ncols();
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numRows; c++) {
			double value = temp_vect.get(r, c);
			SingVals->put(r, c, value);
		}
	}
	
}

AlgorithmOutput ComputeSVDAlgo::run_generic(const AlgorithmInput& input) const
{
	auto input_matrix = input.get<Matrix>(Variables::InputMatrix);
	
	matrix_convert::to_dense(input_matrix);
	
	DenseMatrixHandle LeftSingMat, RightSingMat;
	DenseColumnMatrixHandle SingVals;
	
	run(input_matrix, LeftSingMat, SingVals, RightSingMat);
	
	AlgorithmOutput output;
	
	output[LeftSingularMatrix] = LeftSingMat;
	output[SingularValues] = SingVals;
	output[RightSingularMatrix] = RightSingMat;
	
	return output;
}