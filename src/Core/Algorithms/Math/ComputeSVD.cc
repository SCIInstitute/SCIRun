/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <Externals/eigen/Eigen/src/SVD/JacobiSVD.h>
#include <Core/Datatypes/MatrixFwd.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

void  ComputeSVDAlgorithm::run(MatrixHandle output_matrix, DenseMatrixHandle LeftSingMat, DenseMatrixHandle RightSingMat, DenseColumnMatrixHandle SingVals) const
{
	// Make sure that input is a dense matrix.
	//if(matrix_is::sparse(output_matrix))
	matrix_convert::to_dense(output_matrix);
		
	int numRows = output_matrix->nrows();
	int numCols = output_matrix->ncols();
	
	//DenseMatrixHandle temp;
	
	//output_matrix->to(temp);
	//Datatypes::DenseMatrix temp;
	
	
	/*
	for(int i = 0; i < 10; i++) {
		for(int j = 0; j < 10; j++) {
			temp.put(i,j,i + j);
		}
	}

	*/
	
	bool temp1 = temp.computeU();
	
	//LeftSingMat = output_matrix.matrixU();
	//RightSingMat = output_matrix.matrixV();
	//SingVals = output_matrix.singularValue();
	
	
	
	/*
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			temp(r,c) = output_matrix->get(r,c);
		}
	}
		
	JacobiSVD<EigenBase> SVD_matrix();
	SVD_matrix = SVD_matrix.compute(temp, ComputeFullU | ComputeFullV);
	
	MatrixHandle return_matrix;
	
	numRows = SVD_matrix.rows();
	numCols = SVD_matrix.cols();
	
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			return_matrix->put(
	*/
	
}


AlgorithmOutput ComputeSVDAlgorithm::run_generic(const AlgorithmInput& input) const
{
	auto input_matrix = input.get<Matrix>(Variables::InputMatrix);
	
	auto output_matrix = input_matrix;
	
	DenseMatrixHandle LeftSingMat, RightSingMat;
	DenseColumnMatrixHandle SingVals;
	
	run(output_matrix, LeftSingMat, RightSingMat, SingVals);
	
	AlgorithmOutput output;
	output[LeftSingularMatrix] = LeftSingMat;
	output[RightSingularMatrix] = RightSingMat;
	output[SingularValues] = SingVals;
	
	return output;
}











