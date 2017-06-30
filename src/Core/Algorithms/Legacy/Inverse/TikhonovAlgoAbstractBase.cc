/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

Author            : Jaume Coll-Font
Last modification : April 20 2017
*/

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

// Tikhonov specific headers
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovImpl.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithStandardTikhonovImpl.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD_impl.h>

// Datatypes
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Math/MiscMath.h>

// SCIRun structural
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Inverse;

// shared inputs
const AlgorithmInputName TikhonovAlgoAbstractBase::ForwardMatrix("ForwardMatrix");
const AlgorithmInputName TikhonovAlgoAbstractBase::MeasuredPotentials("MeasuredPotentials");
const AlgorithmInputName TikhonovAlgoAbstractBase::WeightingInSourceSpace("WeightingInSourceSpace");
const AlgorithmInputName TikhonovAlgoAbstractBase::WeightingInSensorSpace("WeightingInSensorSpace");

// Inputs specific from the Tikhonov SVD module
const AlgorithmInputName TikhonovAlgoAbstractBase::matrixU("matrixU");
const AlgorithmInputName TikhonovAlgoAbstractBase::singularValues("singularValues");
const AlgorithmInputName TikhonovAlgoAbstractBase::matrixV("matrixV");

// outputs
const AlgorithmOutputName TikhonovAlgoAbstractBase::InverseSolution("InverseSolution");
const AlgorithmOutputName TikhonovAlgoAbstractBase::RegularizationParameter("RegularizationParameter");
const AlgorithmOutputName TikhonovAlgoAbstractBase::RegInverse("RegInverse");

ALGORITHM_PARAMETER_DEF( Inverse, TikhonovImplementation);
ALGORITHM_PARAMETER_DEF( Inverse, RegularizationMethod);
ALGORITHM_PARAMETER_DEF( Inverse, regularizationChoice);
ALGORITHM_PARAMETER_DEF( Inverse, LambdaFromDirectEntry);
ALGORITHM_PARAMETER_DEF( Inverse, LambdaMin);
ALGORITHM_PARAMETER_DEF( Inverse, LambdaMax);
ALGORITHM_PARAMETER_DEF( Inverse, LambdaNum);
ALGORITHM_PARAMETER_DEF( Inverse, LambdaResolution);
ALGORITHM_PARAMETER_DEF( Inverse, LambdaSliderValue);
ALGORITHM_PARAMETER_DEF( Inverse, LambdaCorner);
ALGORITHM_PARAMETER_DEF( Inverse, LCurveText);
ALGORITHM_PARAMETER_DEF( Inverse, regularizationSolutionSubcase);
ALGORITHM_PARAMETER_DEF( Inverse, regularizationResidualSubcase);

TikhonovAlgoAbstractBase::TikhonovAlgoAbstractBase()
{
 	using namespace Parameters;

	addParameter(Parameters::TikhonovImplementation, std::string("NoMethodSelected") );
	addOption(Parameters::RegularizationMethod, "lcurve", "single|slider|lcurve");
	addParameter(Parameters::regularizationChoice, 0);
	addParameter(Parameters::LambdaFromDirectEntry,1e-6);
	addParameter(Parameters::LambdaMin,1e-6);
	addParameter(Parameters::LambdaMax,1);
	addParameter(Parameters::LambdaNum,200);
	addParameter(Parameters::LambdaResolution,1e-6);
	addParameter(Parameters::LambdaSliderValue,0);
	addParameter(Parameters::LambdaCorner,0);
	addParameter(Parameters::LCurveText,"lcurve");
	addParameter(Parameters::regularizationSolutionSubcase,solution_constrained);
	addParameter(Parameters::regularizationResidualSubcase,residual_constrained);

}

////// CHECK IF INPUT MATRICES HAVE THE CORRECT SIZE
bool TikhonovAlgoAbstractBase::checkInputMatrixSizes( const AlgorithmInput & input) const
{
	// get inputs
	auto forwardMatrix_ = input.get<Matrix>(ForwardMatrix);
	auto measuredData_ = input.get<Matrix>(MeasuredPotentials);
	auto sourceWeighting_ = input.get<Matrix>(WeightingInSourceSpace);
	auto sensorWeighting_ = input.get<Matrix>(WeightingInSensorSpace);

	// get input sizes
    const int M = forwardMatrix_->nrows();
    const int N = forwardMatrix_->ncols();

    // check that rows of fwd matrix equal number of measurements
    if ( M != measuredData_->nrows() )
    {
		THROW_ALGORITHM_INPUT_ERROR("Input matrix dimensions must agree.");
		return false;
    }

    // check that number of time samples is 1. @JCOLLFONT to change for a more general case later (should add a for loop)
    if (1 != measuredData_->ncols())
    {
		THROW_ALGORITHM_INPUT_ERROR("Measured data must be a vector");
		return false;
    }

    // check source regularization matrix sizes
    if (sourceWeighting_)
    {
        if( get(Parameters::regularizationSolutionSubcase).toInt()==solution_constrained )
        {
            // check that the matrix is of appropriate size (equal number of rows as columns in fwd matrix)
            if ( N != sourceWeighting_->ncols() )
            {
				THROW_ALGORITHM_INPUT_ERROR("Solution Regularization Matrix must have the same number of rows as columns in the Forward Matrix !");
				return false;
            }
        }
        // otherwise, if the source regularization is provided as the squared version (RR^T)
        else if ( get(Parameters::regularizationSolutionSubcase).toInt()==solution_constrained_squared )
        {
            // check that the matrix is of appropriate size and squared (equal number of rows as columns in fwd matrix)
            if ( ( N != sourceWeighting_->nrows() ) || ( N != sourceWeighting_->ncols() ) )
            {
				THROW_ALGORITHM_INPUT_ERROR("The squared solution Regularization Matrix must have the same number of rows and columns and must be equal to the number of columns in the Forward Matrix !");
				return false;
            }
        }
    }

    // check measurement regularization matrix sizes
    if (sensorWeighting_)
    {
        if (get(Parameters::regularizationResidualSubcase).toInt() == residual_constrained)
        {
            // check that the matrix is of appropriate size (equal number of rows as rows in fwd matrix)
            if(M != sensorWeighting_->ncols())
            {
				THROW_ALGORITHM_INPUT_ERROR("Data Residual Weighting Matrix must have the same number of rows as the Forward Matrix !");
				return false;
            }
        }
        // otherwise if the source covariance matrix is provided in squared form
        else if  ( get(Parameters::regularizationResidualSubcase).toInt() == residual_constrained_squared )
        {
            // check that the matrix is of appropriate size and squared (equal number of rows as rows in fwd matrix)
            if( (M != sensorWeighting_->nrows()) || (M != sensorWeighting_->ncols()) )
            {
				THROW_ALGORITHM_INPUT_ERROR("Squared data Residual Weighting Matrix must have the same number of rows and columns as number of rows in the Forward Matrix !");
				return false;
			}

        }
    }

    return true;

}


/////////////////////////
/////////  run()
AlgorithmOutput TikhonovAlgoAbstractBase::run(const AlgorithmInput & input) const
{
	// get inputs
	auto forwardMatrix_ = input.get<Matrix>(TikhonovAlgoAbstractBase::ForwardMatrix);
	auto measuredData_ = input.get<Matrix>(TikhonovAlgoAbstractBase::MeasuredPotentials);
	auto sourceWeighting_ = input.get<Matrix>(TikhonovAlgoAbstractBase::WeightingInSourceSpace);
	auto sensorWeighting_ = input.get<Matrix>(TikhonovAlgoAbstractBase::WeightingInSensorSpace);

	// get Parameters
	auto RegularizationMethod_gotten = getOption(Parameters::RegularizationMethod);
	auto TikhonovImplementation_gotten = get(Parameters::TikhonovImplementation).toString();

    // Alocate Variable
	DenseMatrix solution;
    double lambda_sq = 0;
	double lambda_ = 0;

	// check input MATRICES
	checkInputMatrixSizes( input );

	// Determine specific Tikhonov Implementation
	TikhonovImpl  *algoImpl;
	if ( TikhonovImplementation_gotten ==  std::string("standardTikhonov") ){

		// get Parameters
		int  regularizationChoice_ = get(Parameters::regularizationChoice).toInt();
		int regularizationSolutionSubcase_ = get(Parameters::regularizationSolutionSubcase).toInt();
		int regularizationResidualSubcase_ = get(Parameters::regularizationResidualSubcase).toInt();

		algoImpl = new SolveInverseProblemWithStandardTikhonovImpl( *castMatrix::toDense(forwardMatrix_), *castMatrix::toDense(measuredData_), *castMatrix::toDense(sourceWeighting_), *castMatrix::toDense(sensorWeighting_), regularizationChoice_, regularizationSolutionSubcase_, regularizationResidualSubcase_);
	}
	else if ( TikhonovImplementation_gotten ==  std::string("TikhonovSVD") ){

		// get Parameters
		int  regularizationChoice_ = get(Parameters::regularizationChoice).toInt();
		int regularizationSolutionSubcase_ = get(Parameters::regularizationSolutionSubcase).toInt();
		int regularizationResidualSubcase_ = get(Parameters::regularizationResidualSubcase).toInt();

		// get TikhonovSVD special inputs
		auto matrixU_ = input.get<Matrix>(TikhonovAlgoAbstractBase::matrixU);
		auto singularValues_ = input.get<Matrix>(TikhonovAlgoAbstractBase::singularValues);
		auto matrixV_ = input.get<Matrix>(TikhonovAlgoAbstractBase::matrixV);

		algoImpl = new SolveInverseProblemWithTikhonovSVD_impl( *castMatrix::toDense(forwardMatrix_), *castMatrix::toDense(measuredData_), *castMatrix::toDense(sourceWeighting_), *castMatrix::toDense(sensorWeighting_), *castMatrix::toDense(matrixU_), *castMatrix::toDense(singularValues_), *castMatrix::toDense(matrixV_) );
	}
	else if ( TikhonovImplementation_gotten ==  std::string("TikhonovTSVD") ){
		THROW_ALGORITHM_PROCESSING_ERROR("Tikhonov TSVD not implemented yet");
	}
	else{
		THROW_ALGORITHM_PROCESSING_ERROR("Not a valid Tikhonov Implementation selection");
	}

	// preAlocateInverseMatrices(forwardMatrix_,measuredData_,sourceWeighting_,sensorWeighting_);

    //Get Regularization parameter(s) : Lambda
    if ((RegularizationMethod_gotten == "single") || (RegularizationMethod_gotten == "slider"))
    {
        if (RegularizationMethod_gotten == "single")
        {
            // Use single fixed lambda value, entered in UI
            lambda_ = get(Parameters::LambdaFromDirectEntry).toDouble();
        }
        else if (RegularizationMethod_gotten == "slider")
        {
            // Use single fixed lambda value, select via slider
            lambda_ = get(Parameters::LambdaSliderValue).toDouble();
        }
    }
    else if (RegularizationMethod_gotten == "lcurve")
    {
        lambda_ = computeLcurve( algoImpl, input );
    }
	else
	{
		THROW_ALGORITHM_PROCESSING_ERROR("Lambda selection was never set");
	}

	std::cout << "Lambda: "  << lambda_ << std::endl;
    lambda_sq = lambda_ * lambda_;


    // compute inverse solution
	solution = algoImpl->computeInverseSolution(lambda_sq, true);

	//
    // // set final result
    // inverseSolution_.reset(new DenseMatrix(solution));

    // output regularization parameter
    // DenseColumnMatrix tempLambda(1);
    // tempLambda[0] = lambda_;
	//
    // regularizationParameter_. reset( new DenseColumnMatrix(tempLambda) );


	// Set outputs
	AlgorithmOutput output;
	output[InverseSolution] = boost::make_shared<DenseMatrix>(solution);
	output[RegularizationParameter] = boost::make_shared<DenseMatrix>(lambda_);

	return output;

}
//////// fi  run()
///////////////////////////


///////////////////////////
/////// compute L-curve
double TikhonovAlgoAbstractBase::computeLcurve( const SCIRun::Core::Algorithms::Inverse::TikhonovImpl * algoImpl, const AlgorithmInput & input ) const
{
	// get inputs
	auto forwardMatrix_ = input.get<Matrix>(TikhonovAlgoAbstractBase::ForwardMatrix);
	auto measuredData_ = input.get<Matrix>(TikhonovAlgoAbstractBase::MeasuredPotentials);
	auto sourceWeighting_ = input.get<Matrix>(TikhonovAlgoAbstractBase::WeightingInSourceSpace);
	auto sensorWeighting_ = input.get<Matrix>(TikhonovAlgoAbstractBase::WeightingInSensorSpace);
    // define the step size of the lambda vector to be computed  (distance between min and max divided by number of desired lambdas in log scale)
    const int nLambda = get(Parameters::LambdaNum).toInt();
	const double lambdaMin_ = get(Parameters::LambdaMin).toDouble();
	const double lambdaMax_ = get(Parameters::LambdaMax).toDouble();
    const double lam_step = (log10(lambdaMax_) - log10(lambdaMin_))  / (nLambda-1);
	std::cout << "Lambda power step: " << lam_step << ". Number: "<< nLambda <<". Lambda min: " << lambdaMin_ << ". Lambda max: "<< lambdaMax_<< ". Ratio: "<<  lambdaMax_ / lambdaMin_ << std::endl;
    double lambda = 0;
    double lambda_sq;

    // prealocate vector of lambdas and eta and rho
    std::vector<double> lambdaArray(nLambda, 0.0);
    std::vector<double> rho(nLambda, 0.0);
    std::vector<double> eta(nLambda, 0.0);

    DenseMatrix CAx, Rx;
    DenseMatrix solution;

    lambdaArray[0] = lambdaMin_;

    // for all lambdas
    for (int j = 0; j < nLambda; j++)
    {
        if (j)
        {
            lambdaArray[j] = lambdaArray[j-1] * pow(10.0,lam_step);
        }

        // set current lambda
        lambda_sq = lambdaArray[j] * lambdaArray[j];

        // COMPUTE INVERSE SOLUTION
        solution = algoImpl->computeInverseSolution( lambda_sq, false);


        // if using source regularization matrix, apply it to compute Rx (for the eta computations)
        if (sourceWeighting_)
        {
            if (solution.nrows() == sourceWeighting_->ncols()) // check that regularization matrix and solution match sizes
                Rx = (*castMatrix::toDense(sourceWeighting_)) * solution;
            else
            {
				BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage(" Solution weighting matrix unexpectedly does not fit to compute the weighted solution norm. "));
            }
        }
        else
            Rx = solution;


        auto Ax = (*castMatrix::toDense(forwardMatrix_)) * solution;
        auto residualSolution = Ax - (*castMatrix::toDense(measuredData_));

        // if using source regularization matrix, apply it to compute Rx (for the eta computations)
        if (sensorWeighting_)
            CAx = (*castMatrix::toDense(sensorWeighting_)) * residualSolution;
        else
            CAx = residualSolution;


        // compute rho and eta
        rho[j]=0; eta[j]=0;
        for (int k = 0; k < CAx.nrows(); k++)
        {
            double T = CAx(k);
            rho[j] += T*T; //norm of the data fit term
        }

        for (int k = 0; k < Rx.nrows(); k++)
        {
            double T = Rx(k);
            eta[j] += T*T; //norm of the model term
        }

        // eta and rho needed to plot the Lcurve and determine the L corner
        rho[j] = sqrt(rho[j]);
        eta[j] = sqrt(eta[j]);


    }

    // Find corner in L-curve
    lambda = FindCorner( rho, eta, lambdaArray, nLambda );

    // // update GUI
    // if (updateLCurveGui_)
    //     updateLCurveGui_(lambda, *lcurveInput_handle_, lambda_index);

    // return lambda
    return lambda;

}
//////// fi compute L-curve
/////////////////////////////


///// Find Corner, find the maximal curvature which corresponds to the L-curve corner
double TikhonovAlgoAbstractBase::FindCorner( const std::vector<double>& rho, const std::vector<double>& eta, const std::vector<double>& lambdaArray, const int nLambda )
{
	int lambda_index;
    std::vector<double> deta(nLambda);
    std::vector<double> ddeta(nLambda);
    std::vector<double> drho(nLambda);
    std::vector<double> ddrho(nLambda);
    std::vector<double> lrho(nLambda);
    std::vector<double> leta(nLambda);
    DenseColumnMatrix kapa(nLambda);

    double maxKapa = -1.0e10;
    for (int i = 0; i < nLambda; i++)
    {
        lrho[i] = std::log10(rho[i]);
        leta[i] = std::log10(eta[i]);
        if(i>0)
        {
            deta[i] = (leta[i]-leta[i-1]) / (lambdaArray[i]-lambdaArray[i-1]); // compute first derivative
            drho[i] = (lrho[i]-lrho[i-1]) / (lambdaArray[i]-lambdaArray[i-1]);
        }
        if(i>1)
        {
            ddeta[i] = (deta[i]-deta[i-1]) / (lambdaArray[i]-lambdaArray[i-1]); // compute second derivative from first
            ddrho[i] = (drho[i]-drho[i-1]) / (lambdaArray[i]-lambdaArray[i-1]);
        }
    }
    drho[0] = drho[1];
    deta[0] = deta[1];
    ddrho[0] = ddrho[2];
    ddrho[1] = ddrho[2];
    ddeta[0] = ddeta[2];
    ddeta[1] = ddeta[2];

    lambda_index = 0;
    for (int i = 0; i < nLambda; i++)
    {
        kapa[i] = std::abs((drho[i] * ddeta[i] - ddrho[i] * deta[i]) /  //compute curvature
                           std::sqrt(std::pow((deta[i]*deta[i]+drho[i]*drho[i]), 3.0)));
        if (kapa[i] > maxKapa) // find max curvature
        {
            maxKapa = kapa[i];
            lambda_index = i;
        }
    }

    return lambdaArray[lambda_index];
}

// ///// Search for closest Lambda to given lambda
// double TikhonovAlgoAbstractBase::LambdaLookup( LCurveInput& Linput, double lambda, int& lambda_index, const double epsilon)
// {
// 	const int nLambda = Linput.nLambda_;
// 	const std::vector<double>& lambdaArray = Linput.lambdaArray_;
//
//     for (int i = 0; i < nLambda-1; ++i)
//     {
//         if (i > 0 && (lambda < lambdaArray[i-1] || lambda > lambdaArray[i+1])) continue;
//
//         double lambda_step_midpoint = std::abs(lambdaArray[i+1] - lambdaArray[i])/2;
//
//         if (std::abs(lambda - lambdaArray[i]) <= epsilon)  // TODO: is this a reasonable comparison???
//         {
//             lambda_index = i;
//             return lambdaArray[lambda_index];
//         }
//
//         if (std::abs(lambda - lambdaArray[i]) < lambda_step_midpoint)
//         {
//             lambda_index = i;
//             return lambdaArray[lambda_index];
//         }
//
//         if (std::abs(lambda - lambdaArray[i+1]) < lambda_step_midpoint)
//         {
//             lambda_index = i+1;
//             return lambdaArray[lambda_index];
//         }
//     }
//     return -1;
// }

// ////////// update L-curve graph
// void TikhonovAlgoAbstractBase::update_graph( LCurveInput & input, double lambda, int lambda_index, const double epsilon)
// {
//
//     if (lcurveInput_handle_ && input.updateLCurveGui_)
//     {
//         lambda = LambdaLookup(*lcurveInput_handle_, lambda, lambda_index, epsilon);
//         if (lambda >= 0)
//         {
//             input.updateLCurveGui_(lambda, *lcurveInput_handle_, lambda_index);
//         }
//     }
// }
