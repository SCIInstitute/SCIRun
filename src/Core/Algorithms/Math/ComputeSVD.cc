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
//#include <Externals/eigen/Eigen/src/SVD/JacobiSVD.h>
//#include <Core/Datatypes/MatrixFwd.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

void  ComputeSVDAlgorithm::run(MatrixHandle output_matrix, DenseMatrixHandle LeftSingMat, DenseMatrixHandle RightSingMat, DenseColumnMatrixHandle SingVals) const
{
	// Make sure that input is a dense matrix.
	if(matrix_is::sparse(output_matrix))
		matrix_convert::to_dense(output_matrix);
		
	int numRows = output_matrix->nrows();
	int numCols = output_matrix->ncols();
	
	//Eigen::JacobiSVD<Eigen::MatrixXf> temp(numRows
	Eigen::JacobiSVD<DenseMatrixGeneric<double>::EigenBase> temp(output_matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);

	DenseMatrix test1 = temp.matrixV();
	
	numRows = test1.nrows();
	numCols = test1.ncols();
	
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			double value = test1.get(r, c);
			LeftSingMat->put(r, c, value);
		}
	}
	
	test1 = temp.matrixU();
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			double value = test1.get(r, c);
			RightSingMat->put(r, c, value);
		}
	}
	
	DenseColumnMatrix test2 = temp.singularValues();
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			double value = test1.get(r, c);
			SingVals->put(r, c, value);
		}
	}
	
	
	
	 //matrix_convert::to_dense(test1);
	
	//matrix_convert::to_dense(temp);
	//matrix_cast::to<DenseMatrix>(temp);
	
	//LeftSingMat = matrix_convert::to_dense(temp.matrixU());
	//test2 = temp.matrixV();
	//test3 = temp.singularValues();
	
	int x = temp.rows();
	int y = temp.cols();
	
	//std::cout << temp.get(1,2) << std::endl;
	
	//DenseMatrixHandle k = ;
    auto p = output_matrix;
	p->MatrixBase::put(1,2,4);
	p->MatrixBase::get(1,2);
	
	//Eigen::JacobiSVD(output_matrix);
	
	
	for(int r = 0; r < numRows; r++) {
		for(int c = 0; c < numCols; c++) {
			double value = output_matrix->get(r,c);
			//temp.DenseMatrixGeneric<double>::put(r,c,value);
		}
	}
	DenseMatrixGeneric<double>::EigenBase test = temp.matrixU();
	
	
	
	
	
	/*
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
	output[Variables::LeftSingularMatrix] = LeftSingMat;
	output[Variables::RightSingularMatrix] = RightSingMat;
	output[Variables::SingularValues] = SingVals;
	
	return output;
}











