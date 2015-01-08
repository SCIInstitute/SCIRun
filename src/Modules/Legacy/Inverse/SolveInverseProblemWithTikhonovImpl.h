/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#include <Core/Utils/ProgressReporter.h>

#include <Modules/Legacy/Inverse/share.h>

namespace BioPSE
{
  // TODO: this needs to be moved into a SCIRun algorithm class
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

    TikhonovAlgorithmImpl(const SCIRun::Core::Datatypes::MatrixHandle& forwardMatrix,
                          const SCIRun::Core::Datatypes::MatrixHandle& measuredData,
                          AlgorithmChoice regularizationChoice = automatic,
                          AlgorithmSolutionSubcase regularizationSolutionSubcase = solution_constrained,
                          AlgorithmResidualSubcase regularizationResidualSubcase = residual_constrained,
                          const SCIRun::Core::Datatypes::MatrixHandle sourceWeighting = 0,
                          const SCIRun::Core::Datatypes::MatrixHandle sensorWeighting = 0,
                          bool computeRegularizedInverse = false,
                          SCIRun::Core::Utility::ProgressReporter* pr = 0);
    ~TikhonovAlgorithmImpl();

    SCIRun::Core::Datatypes::MatrixHandle get_inverse_solution() const;
    SCIRun::Core::Datatypes::MatrixHandle get_inverse_matrix() const;
    SCIRun::Core::Datatypes::DenseColumnMatrixHandle get_regularization_parameter() const;

    struct SCISHARE LCurveInput
    {
      const std::vector<double> rho_;
      const std::vector<double> eta_;
      const std::vector<double> lambdaArray_;
      const int nLambda_;

      LCurveInput(const std::vector<double>& rho, const std::vector<double>& eta, const std::vector<double>& lambdaArray, const int nLambda);
    };

    struct SCISHARE Input
    {
      std::string regMethod_;
      double lambdaFromTextEntry_;
      double lambdaSlider_;
      int lambdaCount_;
      double lambdaMin_;
      double lambdaMax_;

      typedef boost::function<void (double lambda, const LCurveInput& input, int lambda_index)> lcurveGuiUpdate;
      lcurveGuiUpdate updateLCurveGui_;

      Input(const std::string& regMethod, double lambdaFromTextEntry, double lambdaSlider, int lambdaCount,
            double lambdaMin, double lambdaMax, lcurveGuiUpdate updateLCurveGui);
    };

    void run(const Input& input);
    void update_graph(const TikhonovAlgorithmImpl::Input& input, double lambda, int lambda_index, const double epsilon);

    static double FindCorner(const LCurveInput& input, int& lambda_index);
    static double LambdaLookup(const LCurveInput& input, double lambda, int& lambda_index, const double epsilon);

  private:
    const SCIRun::Core::Datatypes::MatrixHandle& forwardMatrix_;
    const SCIRun::Core::Datatypes::MatrixHandle& measuredData_;
    const SCIRun::Core::Datatypes::MatrixHandle sourceWeighting_;
    const SCIRun::Core::Datatypes::MatrixHandle sensorWeighting_;

    SCIRun::Core::Datatypes::MatrixHandle inverseSolution_;
    SCIRun::Core::Datatypes::MatrixHandle inverseMatrix_;
    SCIRun::Core::Datatypes::DenseColumnMatrixHandle regularizationParameter_;

    AlgorithmChoice regularizationChoice_;
    AlgorithmSolutionSubcase regularizationSolutionSubcase_;
    AlgorithmResidualSubcase regularizationResidualSubcase_;

    double lambda_;
    bool computeRegularizedInverse_;
    boost::shared_ptr<LCurveInput> lcurveInput_handle_;
    SCIRun::Core::Utility::ProgressReporter* pr_;
  };
}

#endif
