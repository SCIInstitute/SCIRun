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

   Author:              Jaume Coll-Font, Moritz Dannhauer, Ayla Khan, Dan White
   Last Modification:   September 6 2017
*/


#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithStandardTikhonovImpl.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <Core/Logging/LoggerInterface.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Inverse;


/////////////////////////
///////// compute Inverse solution
    DenseMatrix SolveInverseProblemWithStandardTikhonovImpl::computeInverseSolution( double lambda, bool inverseCalculation) const
    {
        //............................
        //  OPERATIONS PERFORMED IN THIS SECTION:
        //      The description of these operations is general and applies to underdetermined or overdetermined equations depending on the definition given to M1, M2, M3 and y (look at the selection of underdetermined or overdetermined for details)
        //............................
        //
        //      G = (M1 + lambda^2 * M2)
        //      b = G^-1 * y
        //      x = M3 * b
        //
        //      A^-1 = M3 * G^-1 * M4
        //...........................................................................................................
        const int sizeB = M1.ncols();
        const int sizeSolution = M3.nrows();
        const int numTimeSamples = y.ncols();
        DenseMatrix inverseG(sizeB,sizeB);

        DenseMatrix b(sizeB);
        DenseMatrix solution(sizeSolution,numTimeSamples);
        DenseMatrix G;

        G = M1 + lambda * lambda * M2;

        b = G.lu().solve(y).eval();

        solution = M3 * b;

        // if (inverseCalculation)
        // {
        //     inverseG = G.inverse().eval();
        //     inverseMatrix_.reset( new DenseMatrix( (M3 * inverseG) * M4) );
        // }
        //     inverseSolution_.reset(new DenseMatrix(solution));
        return solution;
    }
//////// fi compute inverse solution
////////////////////////

/////// precomputeInverseMatrices
///////////////
    void SolveInverseProblemWithStandardTikhonovImpl::preAlocateInverseMatrices(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_, const SCIRun::Core::Datatypes::DenseMatrix& measuredData_ , const SCIRun::Core::Datatypes::DenseMatrix& sourceWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& sensorWeighting_, const int regularizationChoice_, const int regularizationSolutionSubcase_, const int regularizationResidualSubcase_)
    {

        // TODO: use DimensionMismatch exception where appropriate
        // DIMENSION CHECK!!
        const int M = forwardMatrix_.nrows();
        const int N = forwardMatrix_.ncols();

        // PREALOCATE VARIABLES and MATRICES
        DenseMatrix forward_transpose = forwardMatrix_.transpose();

		// get Parameters
		// auto  regularizationChoice_ = get(regularizationChoice).toInt();
		// auto regularizationChoice_ = get(regularizationSolutionSubcase).toInt();
		// auto regularizationResidualSubcase_ = get(regularizationResidualSubcase).toInt();

        // select underdetermined case if user decides so or the option is set to automatic and number of measurements is smaller than number of unknowns.
        if ( ( (M < N) && (regularizationChoice_ ==  TikhonovAlgoAbstractBase::automatic) ) || (regularizationChoice_ ==  TikhonovAlgoAbstractBase::underdetermined))
        {
            //UNDERDETERMINED CASE
            //.........................................................................
            // OPERATE ON DATA:
            // Compute X = (R^T * R)^-1 * A^T (A * (R^T*R)^-1 * A^T + LAMBDA * LAMBDA * (C^T*C)^-1 ) * Y
            //         X = M3                *              G^-1                                 * (M4) * Y
            // Will set:
            //      M1 = A * (R^T*R)^-1 * A^T
            //      M2 = (C^T*C)^-1
            //      M3 = (R * R^T)^-1 * A^T
            //      M4 = identity
            //      y = measuredData
            //.........................M1,................................................

            DenseMatrix RRtr(N,N);
            DenseMatrix iRRtr(N,N);
            DenseMatrix CCtr(M,M);
            DenseMatrix iCCtr(M,M);

            // DEFINITIONS AND PREALOCATION OF SOURCE REGULARIZATION MATRIX 'R'
            // if R does not exist, set as identity of size equal to N (columns of fwd matrix)
            if (true)//(&sourceWeighting_==NULL)
            {
                RRtr = DenseMatrix::Identity(N, N);
                iRRtr = RRtr;
            }
            else
            {

                // if provided the non-squared version of R
                if( regularizationSolutionSubcase_ ==  TikhonovAlgoAbstractBase::solution_constrained )
                {
                    RRtr = sourceWeighting_.transpose() * sourceWeighting_;
                }
                // otherwise, if the source regularization is provided as the squared version (RR^T)
                else if ( regularizationSolutionSubcase_ ==  TikhonovAlgoAbstractBase::solution_constrained_squared )
                {
                    RRtr = sourceWeighting_;
                }

                // check if squared regularization matrix is invertible
				auto LURRtr = RRtr.fullPivLu();
                if ( !LURRtr.isInvertible() )
                {

                    THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Regularization matrix in the source space is not invertible.");
                }

                // COMPUTE inverse
                iRRtr = LURRtr.inverse().eval();

            }


            // DEFINITIONS AND PREALOCATIONS OF MEASUREMENTS COVARIANCE MATRIX 'C'
            // if C does not exist, set as identity of size equal to M (rows of fwd matrix)
            if (true)//(&sensorWeighting_==NULL)
            {
                CCtr = DenseMatrix::Identity(M, M);
                iCCtr = CCtr;

            }
            else
            {
                // if measurement covariance matrix provided in non-squared form
                if (regularizationResidualSubcase_ ==  TikhonovAlgoAbstractBase::residual_constrained)
                {
                    // check that the matrix is of appropriate size (equal number of rows as rows in fwd matrix)
                    if(M != sensorWeighting_.ncols())
                    {
                        CCtr = sensorWeighting_.transpose() * sensorWeighting_;
                    }
                }
                // otherwise if the source covariance matrix is provided in squared form
                else if  ( regularizationResidualSubcase_ ==  TikhonovAlgoAbstractBase::residual_constrained_squared )
                {
                    CCtr = sensorWeighting_;
                }

                // check if squared regularization matrix is invertible
				auto LUCCtr = CCtr.fullPivLu();
                if ( !LUCCtr.isInvertible() )
                {
                    THROW_ALGORITHM_INPUT_ERROR_SIMPLE("Residual covariance matrix is not invertible.");
                }
                iCCtr = LUCCtr.inverse().eval();



            }

            // DEFINE  M1 = (A * (R^T*R)^-1 * A^T MATRIX FOR FASTER COMPUTATION
            DenseMatrix RAtr = iRRtr * forward_transpose;
            M1 = forwardMatrix_ * RAtr;

            // DEFINE M2 = (C^TC)^-1
            M2 = iCCtr;

            // DEFINE M3 = (R^TR)^-1 * A^T
            M3 = RAtr;

            // DEFINE M4 = identity (size of number of measurements)
            M4 = DenseMatrix::Identity(M, N);

            // DEFINE measurement vector
            y = measuredData_;



        }
        //OVERDETERMINED CASE,
        //similar procedure as underdetermined case (documentation comments similar, see above)
        else if ( ( (regularizationChoice_ ==  TikhonovAlgoAbstractBase::automatic) && (M>=N) ) || (regularizationChoice_ ==  TikhonovAlgoAbstractBase::overdetermined) )
        {
            //.........................................................................
            // OPERATE ON DATA:
            // Computes X = (A^T * C^T * C * A + LAMBDA * LAMBDA * R^T * R) * A^T * C^T * C * Y
            //          X = (M3)       *              G^-1                  * M4            * Y
            //.........................................................................
            // Will set:
            //      M1 = A * C^T*C * A^T
            //      M2 = R^T*R
            //      M3 = identity
            //      M4 = A^TC^TC
            //      y = A * C^T*C * measuredData
            //.........................................................................


            // prealocations
            DenseMatrix RtrR(N,N);
            DenseMatrix CtrC(M,M);


            // DEFINITIONS AND PREALOCATION OF SOURCE REGULARIZATION MATRIX 'R'

            // if R does not exist, set as identity of size equal to N (columns of fwd matrix)
            if (true)//(&sourceWeighting_==NULL)
            {
                RtrR = DenseMatrix::Identity(N, N);
            }
            else
            {
                // if provided the non-squared version of R
                if( regularizationSolutionSubcase_ ==  TikhonovAlgoAbstractBase::solution_constrained )
                {
                    RtrR = sourceWeighting_.transpose() * sourceWeighting_;
                }
                // otherwise, if the source regularization is provided as the squared version (RR^T)
                else if (  regularizationSolutionSubcase_ ==  TikhonovAlgoAbstractBase::solution_constrained_squared  )
                {
                    RtrR = sourceWeighting_;
                }
            }


            // DEFINITIONS AND PREALOCATIONS OF MEASUREMENTS COVARIANCE MATRIX 'C'
            // if C does not exist, set as identity of size equal to M (rows of fwd matrix)
            if (true)//(&sensorWeighting_==NULL)
            {
                CtrC = DenseMatrix::Identity(M, M);
            }
            else
            {
                // if measurement covariance matrix provided in non-squared form
                if (regularizationResidualSubcase_ ==  TikhonovAlgoAbstractBase::residual_constrained)
                {
                    CtrC = sensorWeighting_.transpose() * sensorWeighting_;
                }
                // otherwise if the source covariance matrix is provided in squared form
                else if  ( regularizationResidualSubcase_ ==  TikhonovAlgoAbstractBase::residual_constrained_squared )
                {
                    CtrC = sensorWeighting_;
                }

            }

            // DEFINE  M1 = (A * (R*R^T)^-1 * A^T MATRIX FOR FASTER COMPUTATION
            DenseMatrix CtrCA = CtrC * (forwardMatrix_);
            M1 = forward_transpose * CtrCA;

            // DEFINE M2 = (CC^T)^-1
            M2 = RtrR;

            // DEFINE M3 = identity (size of number of measurements)
            M3 = DenseMatrix::Identity(N, N);

            // DEFINT M4 = A^T* C^T * C
            M4 = CtrCA.transpose();

            // DEFINE measurement vector
            y = CtrCA.transpose() * measuredData_;

        }

    }
//////// End of prealocation of matrices
////////////
