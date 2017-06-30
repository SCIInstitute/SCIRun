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

//    File       : SolveInverseProblemWithTikhonovSVD.cc
//    Author     : Yesim Serinagaoglu & Alireza Ghodrati
//    Date       : 07 Aug. 2001
//    Last update: Dec 2011


// SCIRUN lybraries
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Utils/Exception.h>

// Tikhonov inverse libraries
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD_impl.h>

// EIGEN LIBRARY
#include <Eigen/Eigen>
#include <Eigen/SVD>


using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
// using namespace SCIRun::Modules::Inverse;
// using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Inverse;



///////////////////////////////////////////////////////////////////
/////// prealocate Matrices for inverse compuation
///     This function precalcualtes the SVD of the forward matrix and prepares singular vectors and values for posterior computations
///////////////////////////////////////////////////////////////////
void SolveInverseProblemWithTikhonovSVD_impl::preAlocateInverseMatrices(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_, const SCIRun::Core::Datatypes::DenseMatrix& measuredData_ , const SCIRun::Core::Datatypes::DenseMatrix& sourceWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& sensorWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& matrixU_, const SCIRun::Core::Datatypes::DenseMatrix& singularValues_, const SCIRun::Core::Datatypes::DenseMatrix& matrixV_)
{

	// if (!matrixU_.size())&&(!matrixV_.size())&&(!singularValues_.size()){
	// 	std::cout << "Precomputed SVD variables found as an input" << std::endl;
	//
	// 	svd_MatrixU = *matrixU_;
	// 	svd_MatrixV = *matrixV_;
	// 	svd_SingularValues = *singularValues_;
	//
	// }else{

		std::cout << "No precomputed SVD... computing now" << std::endl;

	    // Compute the SVD of the forward matrix
	        Eigen::JacobiSVD<SCIRun::Core::Datatypes::DenseMatrix::EigenBase> SVDdecomposition( forwardMatrix_, Eigen::ComputeFullU | Eigen::ComputeFullV);

			// alocate the left and right singular vectors and the singular values
			svd_MatrixU = SVDdecomposition.matrixU();
			svd_MatrixV = SVDdecomposition.matrixV();
			svd_SingularValues = SVDdecomposition.singularValues();

	    // determine rank
	        rank = SVDdecomposition.nonzeroSingularValues();

	    // Compute the projection of data y on the left singular vectors
	        auto tempUy = SVDdecomposition.matrixU().transpose() * (measuredData_);

	        Uy = tempUy;

	// }
}

//////////////////////////////////////////////////////////////////////
// THIS FUNCTION returns regularized solution by tikhonov method
//////////////////////////////////////////////////////////////////////
SCIRun::Core::Datatypes::DenseMatrix SolveInverseProblemWithTikhonovSVD_impl::computeInverseSolution( double lambda_sq, bool inverseCalculation ) const
{

    // prealocate matrices
        const int N = svd_MatrixV.cols();
        const int M = svd_MatrixU.rows();
        const int numTimeSamples = Uy.ncols();
        DenseMatrix solution(DenseMatrix::Zero(N,numTimeSamples));
        DenseMatrix tempInverse(DenseMatrix::Zero(N,M));

    // Compute inverse solution
        for (int rr=0; rr<rank ; rr++)
        {
            // evaluate filter factor
                double singVal = svd_SingularValues[rr];
                double filterFactor_i =  singVal / ( lambda_sq + singVal * singVal ) * Uy(rr);

            // u[date solution
                solution += filterFactor_i * svd_MatrixV.col(rr);

            // update inverse operator
                if (inverseCalculation)
                    tempInverse += filterFactor_i * ( svd_MatrixV.col(rr) *  svd_MatrixU.col(rr).transpose() );
        }

    // output solutions
    //   if (inverseCalculation)
    //       inverseMatrix_.reset( new SCIRun::Core::Datatypes::DenseMatrix(tempInverse) );

        return solution;
}
