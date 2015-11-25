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

//    File       : SolveInverseProblemWithTikhonov.cc
//    Author     : Moritz Dannhauer, Ayla Khan, Dan White
//    Date       : November 02th, 2012 (last update)

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovImpl.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

#include <Core/Logging/LoggerInterface.h>
#include <Core/Utils/Exception.h>

namespace BioPSE
{

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;

TikhonovAlgorithmImpl::TikhonovAlgorithmImpl(const DenseMatrixHandle& forwardMatrix,
                                             const DenseMatrixHandle& measuredData,
                                             AlgorithmChoice regularizationChoice,
                                             AlgorithmSolutionSubcase regularizationSolutionSubcase,
                                             AlgorithmResidualSubcase regularizationResidualSubcase,
                                             const DenseMatrixHandle sourceWeighting,
                                             const DenseMatrixHandle sensorWeighting,
                                             bool computeRegularizedInverse,
                                             LegacyLoggerInterface* pr)
: forwardMatrix_(forwardMatrix),
measuredData_(measuredData),
sourceWeighting_(sourceWeighting),
sensorWeighting_(sensorWeighting),
regularizationChoice_(regularizationChoice),
regularizationSolutionSubcase_(regularizationSolutionSubcase),
regularizationResidualSubcase_(regularizationResidualSubcase),
lambda_(0),
computeRegularizedInverse_(computeRegularizedInverse),
pr_(pr)
{
  //TODO: size checking here.
}

MatrixHandle TikhonovAlgorithmImpl::get_inverse_solution() const
{
  return inverseSolution_;
}

MatrixHandle TikhonovAlgorithmImpl::get_inverse_matrix() const
{
  return inverseMatrix_;
}

DenseColumnMatrixHandle TikhonovAlgorithmImpl::get_regularization_parameter() const
{
  return regularizationParameter_;
}

//! Find Corner, find the maximal curvature which corresponds to the L-curve corner
double
TikhonovAlgorithmImpl::FindCorner(const TikhonovAlgorithm::LCurveInput& input, int& lambda_index)
{
  const std::vector<double>& rho = input.rho_;
  const std::vector<double>& eta = input.eta_;
  const std::vector<double>& lambdaArray = input.lambdaArray_;
  int nLambda = input.nLambda_;

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

double
TikhonovAlgorithmImpl::LambdaLookup(const TikhonovAlgorithm::LCurveInput& input, double lambda, int& lambda_index, const double epsilon)
{
  const std::vector<double>& lambdaArray = input.lambdaArray_;
  int nLambda = input.nLambda_;

  for (int i = 0; i < nLambda-1; ++i)
  {
    if (i > 0 && (lambda < lambdaArray[i-1] || lambda > lambdaArray[i+1])) continue;

    double lambda_step_midpoint = std::abs(lambdaArray[i+1] - lambdaArray[i])/2;

    if (std::abs(lambda - lambdaArray[i]) <= epsilon)  // TODO: is this a reasonable comparison???
    {
      lambda_index = i;
      return lambdaArray[lambda_index];
    }

    if (std::abs(lambda - lambdaArray[i]) < lambda_step_midpoint)
    {
      lambda_index = i;
      return lambdaArray[lambda_index];
    }

    if (std::abs(lambda - lambdaArray[i+1]) < lambda_step_midpoint)
    {
      lambda_index = i+1;
      return lambdaArray[lambda_index];
    }
  }
  return -1;
}

namespace LinearAlgebra
{
  void solve_lapack(const DenseMatrix& A, const DenseColumnMatrix& b, DenseColumnMatrix& x)
  {
    x = A.lu().solve(b).eval();
  }

  class LapackError : public std::exception {};
}

    
    
/////////////////////////
/////////  run()
void TikhonovAlgorithmImpl::run(const TikhonovAlgorithmImpl::Input& input)
{    
    
  // TODO: use DimensionMismatch exception where appropriate
  // DIMENSION CHECK!!
  const int M = forwardMatrix_->nrows();
  const int N = forwardMatrix_->ncols();

// check that rows of fwd matrix equal number of measurements
  if ( M != measuredData_->nrows() )
  {
    BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Input matrix dimensions must agree."));
  }

// check that number of time samples is 1. @JCOLLFONT to change for a more general case later (should add a for loop)
  if (1 != measuredData_->ncols())
  {
    BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Measured data must be a vector"));
  }
  
    
    
// PREALOCATE VARIABLES and MATRICES
    DenseMatrix M1, M2, M3, M4;
    DenseMatrix inverseG(M,N);
    DenseColumnMatrix y;
    DenseMatrix forward_transpose = forwardMatrix_->transpose();
    DenseColumnMatrix solution(M);
    double lambda=0, lambda_sq=0;
    
    
    // select underdetermined case if user decides so or the option is set to automatic and number of measurements is smaller than number of unknowns.
  if ( ((M < N) && (regularizationChoice_ == automatic)) || (regularizationChoice_ == underdetermined))
  {
    //UNDERDETERMINED CASE
    //.........................................................................
    // OPERATE ON DATA:
    // Compute X = (R * R^T)^-1 * A^T (A * (R*R^T)^-1 * A^T + LAMBDA * LAMBDA * (C*C^T)^-1 ) * Y
    //         X = M3                *              G^-1                                 * (M4) * Y
    // Will set:
      //      M1 = A * (R*R^T)^-1 * A^T
      //      M2 = (C*C^T)^-1
      //      M3 = (R * R^T)^-1 * A^T
      //      M4 = identity
      //      y = measuredData
    //.........................................................................
   
    
    DenseMatrix  RRtr, RRtrAtr, CCtr;
    DenseMatrix iRRtr(N,N);
    DenseMatrix iCCtr(M,M);

    // DEFINITIONS AND PREALOCATION OF SOURCE REGULARIZATION MATRIX 'R'

      // if R does not exist, set as identity of size equal to N (columns of fwd matrix)
      if (!sourceWeighting_)
      {
          RRtr = DenseMatrix::Identity(N, N);
          iRRtr = RRtr;
          
      }
      else
      {
          
        // if provided the non-squared version of R
        if( regularizationSolutionSubcase_==solution_constrained )
        {
            
            // check that the matrix is of appropriate size (equal number of rows as columns in fwd matrix)
            if ( N != sourceWeighting_->nrows() )
            {
                BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Solution Regularization Matrix must have the same number of rows as columns in the Forward Matrix !"));
            }
          
            RRtr = *sourceWeighting_ * sourceWeighting_->transpose();
            iRRtr = RRtr.inverse().eval();   // todo: @JCOLLFONT need to compute inverse of RRtr and set try/catch
            
        }
        // otherwise, if the source regularization is provided as the squared version (RR^T)
        else if ( regularizationSolutionSubcase_==solution_constrained_squared )
        {
            // check that the matrix is of appropriate size and squared (equal number of rows as columns in fwd matrix)
            if ( ( N != sourceWeighting_->nrows() ) || ( N != sourceWeighting_->ncols() ) )
            {
                BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("The squared solution Regularization Matrix must have the same number of rows and columns and must be equal to the number of columns in the Forward Matrix !"));
            }
            
            RRtr = *sourceWeighting_;
            iRRtr = RRtr.inverse().eval();   // todo: @JCOLLFONT need to compute inverse of RRtr
            
        }
      }
      
      
    // DEFINITIONS AND PREALOCATIONS OF MEASUREMENTS COVARIANCE MATRIX 'C'
      // if C does not exist, set as identity of size equal to M (rows of fwd matrix)
      if (!sensorWeighting_) 
      {
          CCtr = DenseMatrix::Identity(M, M);
          iCCtr = CCtr;
          
      }
      else
      {
          // if measurement covariance matrix provided in non-squared form
          if (regularizationResidualSubcase_ == residual_constrained) 
          {
              // check that the matrix is of appropriate size (equal number of rows as rows in fwd matrix)
              if(M != sensorWeighting_->nrows())
              {
                   BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Data Residual Weighting Matrix must have the same number of rows as the Forward Matrix !"));
              }
              CCtr = *sensorWeighting_ * sensorWeighting_->transpose();
              iCCtr = CCtr.inverse().eval();
           
          }
          // otherwise if the source covariance matrix is provided in squared form
          else if  ( regularizationResidualSubcase_ == residual_constrained_squared )
          {
              // check that the matrix is of appropriate size and squared (equal number of rows as rows in fwd matrix)
              if( (M != sensorWeighting_->nrows()) && (M != sensorWeighting_->ncols()) )
              {
                  BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Squared data Residual Weighting Matrix must have the same number of rows and columns as number of rows in the Forward Matrix !"));
              }
              
              CCtr = *sensorWeighting_;
              iCCtr = CCtr.inverse().eval();
              
          }
          
      }
      
    // DEFINE  M1 = (A * (R*R^T)^-1 * A^T MATRIX FOR FASTER COMPUTATION
      DenseMatrix RAtr = iRRtr * forward_transpose;
      M1 = *forwardMatrix_ * RAtr;
      
    // DEFINE M2 = (CC^T)^-1
      M2 = iCCtr;
    
    // DEFINE M3 = (RR^T)^-1 * A^T
      M3 = RAtr;
      
    // DEFINE M4 = identity (size of number of measurements)
      M4 = DenseMatrix::Identity(N, N);
      
    // DEFINE measurement vector
      y = *measuredData_;
      
     
  }
  //OVERDETERMINED CASE,
  //similar procedure as underdetermined case (documentation comments similar, see above)
  else if ( ((regularizationChoice_ == automatic) && (M>=N)) || (regularizationChoice_==overdetermined) )
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
        DenseMatrix RtrR, RtrRAtr, CtrC;
        
        
        // DEFINITIONS AND PREALOCATION OF SOURCE REGULARIZATION MATRIX 'R'
        
        // if R does not exist, set as identity of size equal to N (columns of fwd matrix)
        if (!sourceWeighting_)
        {
            RtrR = DenseMatrix::Identity(N, N);
            
        }
        else
        {
            // if provided the non-squared version of R
            if( regularizationSolutionSubcase_==solution_constrained )
            {
                
                // check that the matrix is of appropriate size (equal number of rows as columns in fwd matrix)
                if ( N != sourceWeighting_->nrows() )
                {
                    BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Solution Regularization Matrix must have the same number of rows as columns in the Forward Matrix !"));
                }
                
                RtrR = *sourceWeighting_ * sourceWeighting_->transpose();
                            
            }
            // otherwise, if the source regularization is provided as the squared version (RR^T)
            else if (  regularizationSolutionSubcase_==solution_constrained_squared  )
            {
                // check that the matrix is of appropriate size and squared (equal number of rows as columns in fwd matrix)
                if ( ( N != sourceWeighting_->nrows() ) || ( N != sourceWeighting_->ncols() ) )
                {
                    BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("The squared solution Regularization Matrix must have the same number of rows and columns and must be equal to the number of columns in the Forward Matrix !"));
                }
                
                RtrR = *sourceWeighting_;
                    
            }
        }
        
        
        // DEFINITIONS AND PREALOCATIONS OF MEASUREMENTS COVARIANCE MATRIX 'C'
        // if C does not exist, set as identity of size equal to M (rows of fwd matrix)
        if (!sensorWeighting_)
        {
            CtrC = DenseMatrix::Identity(M, M);
            
        }
        else
        {
            // if measurement covariance matrix provided in non-squared form
            if (regularizationResidualSubcase_ == residual_constrained)
            {
                // check that the matrix is of appropriate size (equal number of rows as rows in fwd matrix)
                if(M != sensorWeighting_->nrows())
                {
                    BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Data Residual Weighting Matrix must have the same number of rows as the Forward Matrix !"));
                }
                CtrC = *sensorWeighting_ * sensorWeighting_->transpose();
                
            }
            // otherwise if the source covariance matrix is provided in squared form
            else if  ( regularizationResidualSubcase_ == residual_constrained_squared )
            {
                // check that the matrix is of appropriate size and squared (equal number of rows as rows in fwd matrix)
                if ( (M != sensorWeighting_->nrows()) && (M != sensorWeighting_->ncols()) )
                {
                    BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo("Squared data Residual Weighting Matrix must have the same number of rows and columns as number of rows in the Forward Matrix !"));
                }
                
                CtrC = *sensorWeighting_;
                
            }
            
        }
        
        // DEFINE  M1 = (A * (R*R^T)^-1 * A^T MATRIX FOR FASTER COMPUTATION
        DenseMatrix CtrCA = CtrC * (*forwardMatrix_);
        M1 = forward_transpose * CtrCA;
        
        // DEFINE M2 = (CC^T)^-1
        M2 = RtrR;
        
        // DEFINE M3 = identity (size of number of measurements)
        M3 = DenseMatrix::Identity(N, N);
      
        // DEFINT M4 = A^T* C^T * C
        M4 = CtrCA.transpose();
      
        // DEFINE measurement vector
        y = CtrCA.transpose() * *measuredData_;
    
  }
    
std::cerr << "gato: precomputations done" << std::endl;
    
///////////////////////////////
// Lambda selection
///////////////////////////////
    
    //Get Regularization parameter(s) : Lambda
    if ((input.regMethod_ == "single") || (input.regMethod_ == "slider"))
    {
      if (input.regMethod_ == "single")
      {
        // Use single fixed lambda value, entered in UI
        lambda = input.lambdaFromTextEntry_;
      }
      else if (input.regMethod_ == "slider")
      {
        // Use single fixed lambda value, select via slider
        lambda = input.lambdaSlider_;
      }
    }
    else if (input.regMethod_ == "lcurve")
    {
        lambda = computeLcurve( input, M1, M2, M3, M4, y );
    }
      
      
    lambda_sq = lambda * lambda;
    
    
/////////////////////////////
// COMPUTE INVERSE SOLUTION  // Todo: @JCOLLFONT function needs to be defined
////////////////////////////
    
    solution = computeInverseSolution( M1, M2, M3, M4, y, lambda_sq, computeRegularizedInverse_);
    
    // set final result
    inverseSolution_.reset(new DenseMatrix(solution));

}
//////// fi  run()
///////////////////////////

///////////////////////////
/////// compute L-curve
double TikhonovAlgorithmImpl::computeLcurve( const TikhonovAlgorithmImpl::Input& input, DenseMatrix& M1, DenseMatrix& M2, DenseMatrix& M3, DenseMatrix& M4, DenseColumnMatrix& y )
{
    
    // define the step size of the lambda vector to be computed  (distance between min and max divided by number of desired lambdas in log scale)
    const int nLambda = input.lambdaCount_;
    const double lam_step = pow(10.0, log10(input.lambdaMax_ / input.lambdaMin_) / (nLambda-1));
    double lambda;
    
    const int sizeSolution = M3.nrows();
    
    double lambda_sq;
    
    // prealocate vector of lambdas and eta and rho
    std::vector<double> lambdaArray(nLambda, 0.0);
    std::vector<double> rho(nLambda, 0.0);
    std::vector<double> eta(nLambda, 0.0);
    
    DenseColumnMatrix CAx, Rx;
    DenseColumnMatrix solution(sizeSolution);
    
    lambdaArray[0] = input.lambdaMin_;
    
    // initialize counter
    int lambda_index = 0;
    
    // for all lambdas
    for (int j = 0; j < nLambda; j++)
    {
        if (j)
        {
            lambdaArray[j] = lambdaArray[j-1] * lam_step;
        }
        
        // set current lambda
        lambda_sq = lambdaArray[j] * lambdaArray[j];
        
        std::cerr << "gato: computing inverse solution in Lcurvce" << std::endl;
        // COMPUTE INVERSE SOLUTION  // Todo: @JCOLLFONT function needs to be defined
        solution = computeInverseSolution( M1, M2, M3, M4, y, lambda_sq, false);
        
        
        // if using source regularization matrix, apply it to compute Rx (for the eta computations)
        if (sourceWeighting_)
        {
            if (solution.nrows() == sourceWeighting_->ncols()) // check that regularization matrix and solution match sizes
                Rx = *sourceWeighting_ * solution;
            else
            {
                const std::string errorMessage(" Solution weighting matrix unexpectedly does not fit to compute the weighted solution norm. ");
                if (pr_)
                    pr_->error(errorMessage);
                else
                    std::cerr << errorMessage << std::endl;
            }
        }
        else
            Rx = solution;
        
        
        auto Ax = *forwardMatrix_  * solution;
        auto residualSolution = Ax - *measuredData_;
        
        // if using source regularization matrix, apply it to compute Rx (for the eta computations)
        if (sensorWeighting_)
            CAx = (*sensorWeighting_) * residualSolution;
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
            double T = Rx[k];
            eta[j] += T*T; //norm of the model term
        }
        
        // eta and rho needed to plot the Lcurve and determine the L corner
        rho[j] = sqrt(rho[j]);
        eta[j] = sqrt(eta[j]);
         
        
    }
    
    // update L-curve
    boost::shared_ptr<TikhonovAlgorithm::LCurveInput> lcurveInput(new TikhonovAlgorithm::LCurveInput(rho, eta, lambdaArray, nLambda));
    lcurveInput_handle_ = lcurveInput;
    
    std::cerr << "gato: finding corner" << std::endl;
    // Find corner in L-curve
    lambda = FindCorner(*lcurveInput_handle_, lambda_index);
    
    // update GUI
    if (input.updateLCurveGui_)
        input.updateLCurveGui_(lambda, *lcurveInput_handle_, lambda_index);
    
    // return lambda
    return lambda;
    
}
//////// fi compute L-curve
/////////////////////////////


/////////////////////////
///////// compute Inverse solution
DenseColumnMatrix TikhonovAlgorithmImpl::computeInverseSolution( DenseMatrix& M1, DenseMatrix& M2, DenseMatrix& M3, DenseMatrix& M4, DenseColumnMatrix& y, double lambda_sq, bool inverseCalculation)
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
    DenseMatrix inverseG(sizeB,sizeB);
    
    DenseColumnMatrix b(sizeB);
    DenseColumnMatrix solution(sizeSolution);
    DenseMatrix G;
    
    std::cerr << "gato: calculating G" << std::endl;
    G = M1 + lambda_sq * M2;
    
    std::cerr << "gato: Solving lapack" << std::endl;
    try
    {
        LinearAlgebra::solve_lapack(G, y, b);
    }
    catch (LinearAlgebra::LapackError&)
    {
        const std::string errorMessage("The Tikhonov linear system could not be solved for a regularization parameter in the Lambda Range of the L-curve. Use a higher Lambda Range ''From'' value for the L-Curve calculation.");
        if (pr_)
        {
            pr_->error(errorMessage);
        }
        else
        {
            std::cerr << errorMessage << std::endl;
        }
        throw;
    }
    catch(DimensionMismatch&)
    {
        const std::string errorMessage("Invalid matrix sizes are being used in the Tikhonov linear system.");
        if (pr_)
        {
            pr_->error(errorMessage);
        }
        else
        {
            std::cerr << errorMessage << std::endl;
        }
        throw;
    }
    
    std::cerr << "gato: calculating final solution" << std::endl;
    
    solution = M3 * b;
    
    std::cerr << "gato: Computing regularized inverse "<< inverseCalculation << std::endl;
    if (inverseCalculation)
    {
        inverseG = G.inverse().eval();
        std::cerr << "gato: setting inverse " << M4.ncols() << " " << M3.nrows() << " " << inverseG.nrows() << " " <<inverseG.ncols() <<   std::endl;
        inverseMatrix_.reset( new DenseMatrix( (M3 * inverseG) * M4) );
    }
    
//     inverseSolution_.reset(new DenseMatrix(solution));
    std::cerr << "gato: returning" << std::endl;
    return solution;
}
//////// fi compute inverse solution
////////////////////////



////////// update graph
void TikhonovAlgorithmImpl::update_graph(const TikhonovAlgorithmImpl::Input& input, double lambda, int lambda_index, const double epsilon)
{
  if (lcurveInput_handle_ && input.updateLCurveGui_)
  {
    lambda = LambdaLookup(*lcurveInput_handle_, lambda, lambda_index, epsilon);
    if (lambda >= 0)
    {
      input.updateLCurveGui_(lambda, *lcurveInput_handle_, lambda_index);
    }
  }
}
    


#if 0

void SolveInverseProblemWithTikhonov::tcl_command(GuiArgs& args, void* userdata)
{
  if (args[1] == "updategraph" && args.count() == 4)
  {
    double lambda = boost::lexical_cast<double>(args[2]);
    int lambda_index = boost::lexical_cast<double>(args[3]);

    if (input_handle_.get() != 0 && algo_handle_.get() != 0)
    {
      algo_handle_->update_graph(*input_handle_, lambda, lambda_index, lambda_resolution_.get());
    }
  }
  else
  {
    // Relay data to the Module class
    Module::tcl_command(args, userdata);
  }
}


#endif

TikhonovAlgorithm::LCurveInput::LCurveInput(const std::vector<double>& rho, const std::vector<double>& eta, const std::vector<double>& lambdaArray, int nLambda)
: rho_(rho), eta_(eta), lambdaArray_(lambdaArray), nLambda_(nLambda)
{}

TikhonovAlgorithmImpl::Input::Input(const std::string& regMethod, double lambdaFromTextEntry, double lambdaSlider, int lambdaCount, double lambdaMin, double lambdaMax,
                                    lcurveGuiUpdate updateLCurveGui)
: regMethod_(regMethod), lambdaFromTextEntry_(lambdaFromTextEntry), lambdaSlider_(lambdaSlider), lambdaCount_(lambdaCount), lambdaMin_(lambdaMin), lambdaMax_(lambdaMax),
updateLCurveGui_(updateLCurveGui)
{}

} // End namespace BioPSE
