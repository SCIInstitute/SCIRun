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
//    Author     : Moritz Dannhauer and Ayla Khan
//    Date       : 15 Aug. 2012

#ifndef BioPSE_SolveInverseProblemWithTikhonov_H__
#define BioPSE_SolveInverseProblemWithTikhonov_H__

#include <vector>

#include <boost/utility.hpp>
#include <boost/function.hpp>

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Logging/LoggerFwd.h>

#include <Modules/Legacy/Inverse/share.h>

namespace BioPSE
{
  // TODO: this needs to be moved into a SCIRun algorithm class
  namespace TikhonovAlgorithm
  { 
    struct SCISHARE LCurveInput
    {
      const std::vector<double> rho_;
      const std::vector<double> eta_;
      const std::vector<double> lambdaArray_;
      const int nLambda_;

      LCurveInput(const std::vector<double>& rho, const std::vector<double>& eta, const std::vector<double>& lambdaArray, const int nLambda);
    };
  }
  
  class SCISHARE TikhonovAlgorithmImpl : boost::noncopyable
  {
  public:
    enum AlgorithmChoice {
      automatic,
      underdetermined,
      overdetermined
    };

    enum AlgorithmSolutionSubcase {
      solution_constrained,
      solution_constrained_squared
    };

    enum AlgorithmResidualSubcase {
      residual_constrained,
      residual_constrained_squared
    };

    TikhonovAlgorithmImpl(const SCIRun::Core::Datatypes::DenseMatrixHandle& forwardMatrix,
                          const SCIRun::Core::Datatypes::DenseMatrixHandle& measuredData,
                          AlgorithmChoice regularizationChoice = automatic,
                          AlgorithmSolutionSubcase regularizationSolutionSubcase = solution_constrained,
                          AlgorithmResidualSubcase regularizationResidualSubcase = residual_constrained,
                          const SCIRun::Core::Datatypes::DenseMatrixHandle sourceWeighting = 0,
                          const SCIRun::Core::Datatypes::DenseMatrixHandle sensorWeighting = 0,
                          bool computeRegularizedInverse = false,
                          SCIRun::Core::Logging::LegacyLoggerInterface* pr = 0);

    SCIRun::Core::Datatypes::MatrixHandle get_inverse_solution() const;
    SCIRun::Core::Datatypes::MatrixHandle get_inverse_matrix() const;
    SCIRun::Core::Datatypes::DenseColumnMatrixHandle get_regularization_parameter() const;

    struct SCISHARE Input
    {
      std::string regMethod_;
      double lambdaFromTextEntry_;
      double lambdaSlider_;
      int lambdaCount_;
      double lambdaMin_;
      double lambdaMax_;

      typedef boost::function<void(double lambda, const TikhonovAlgorithm::LCurveInput& input, int lambda_index)> lcurveGuiUpdate;
      lcurveGuiUpdate updateLCurveGui_;

      Input(const std::string& regMethod, double lambdaFromTextEntry, double lambdaSlider, int lambdaCount,
            double lambdaMin, double lambdaMax, lcurveGuiUpdate updateLCurveGui);
    };

    void run(const Input& input);
    void update_graph(const TikhonovAlgorithmImpl::Input& input, double lambda, int lambda_index, const double epsilon);

    static double FindCorner(const TikhonovAlgorithm::LCurveInput& input, int& lambda_index);
    static double LambdaLookup(const TikhonovAlgorithm::LCurveInput& input, double lambda, int& lambda_index, const double epsilon);

  private:
    const SCIRun::Core::Datatypes::DenseMatrixHandle& forwardMatrix_;
    const SCIRun::Core::Datatypes::DenseMatrixHandle& measuredData_;
    const SCIRun::Core::Datatypes::DenseMatrixHandle sourceWeighting_;
    const SCIRun::Core::Datatypes::DenseMatrixHandle sensorWeighting_;

    SCIRun::Core::Datatypes::DenseMatrixHandle inverseSolution_;
    SCIRun::Core::Datatypes::DenseMatrixHandle inverseMatrix_;
    SCIRun::Core::Datatypes::DenseColumnMatrixHandle regularizationParameter_;

    AlgorithmChoice regularizationChoice_;
    AlgorithmSolutionSubcase regularizationSolutionSubcase_;
    AlgorithmResidualSubcase regularizationResidualSubcase_;

    double lambda_;
    bool computeRegularizedInverse_;
    boost::shared_ptr<TikhonovAlgorithm::LCurveInput> lcurveInput_handle_;
    SCIRun::Core::Logging::LegacyLoggerInterface* pr_;
  };
}

#endif
