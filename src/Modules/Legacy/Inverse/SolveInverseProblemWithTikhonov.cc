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

#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovImpl.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Inverse;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Inverse;

ALGORITHM_PARAMETER_DEF(Inverse, TikhonovSolutionSubcase);
ALGORITHM_PARAMETER_DEF(Inverse, TikhonovResidualSubcase);


const ModuleLookupInfo SolveInverseProblemWithTikhonov::staticInfo_("SolveInverseProblemWithTikhonov", "Inverse", "SCIRun");

SolveInverseProblemWithTikhonov::SolveInverseProblemWithTikhonov() : Module(staticInfo_)
{
  INITIALIZE_PORT(ForwardMatrix);
  INITIALIZE_PORT(WeightingInSourceSpace);
  INITIALIZE_PORT(MeasuredPotentials);
  INITIALIZE_PORT(WeightingInSensorSpace);
  INITIALIZE_PORT(InverseSolution);
  INITIALIZE_PORT(RegularizationParameter);
  INITIALIZE_PORT(RegInverse);
}

void SolveInverseProblemWithTikhonov::setStateDefaults()
{
  auto state = get_state();
  state->setValue(LambdaFromDirectEntry, 0.02);
  state->setValue(RegularizationMethod, std::string("lcurve"));
  state->setValue(LambdaMin, 1e-2);
  state->setValue(LambdaMax, 1e6);
  state->setValue(LambdaNum, 10);
  state->setValue(LambdaResolution, 1e-6);
  state->setValue(LambdaSliderValue, 0.0);
  state->setValue(TikhonovCase, 0);
  state->setValue(Parameters::TikhonovSolutionSubcase, 0);
  state->setValue(Parameters::TikhonovResidualSubcase, 0);
  state->setValue(LambdaCorner, 0.0);
}

void SolveInverseProblemWithTikhonov::execute()
{
  auto forward_matrix_h = getRequiredInput(ForwardMatrix);
  auto hMatrixMeasDat = getRequiredInput(MeasuredPotentials);

  auto hMatrixRegMat = getOptionalInput(WeightingInSourceSpace);
  auto hMatrixNoiseCov = getOptionalInput(WeightingInSensorSpace);

  const bool computeRegularizedInverse = oport_connected(InverseSolution);

  if (needToExecute())
  {
    using namespace BioPSE;
    auto state = get_state();
    auto gui_tikhonov_case = static_cast<TikhonovAlgorithmImpl::AlgorithmChoice>(state->getValue(TikhonovCase).toInt());
    auto gui_tikhonov_solution_subcase = static_cast<TikhonovAlgorithmImpl::AlgorithmSolutionSubcase>(state->getValue(Parameters::TikhonovSolutionSubcase).toInt());
    auto gui_tikhonov_residual_subcase = static_cast<TikhonovAlgorithmImpl::AlgorithmResidualSubcase>(state->getValue(Parameters::TikhonovResidualSubcase).toInt());
    
    auto denseForward = castMatrix::toDense(forward_matrix_h);
    auto measuredDense = convertMatrix::toDense(hMatrixMeasDat);
    auto regMatDense = castMatrix::toDense(hMatrixRegMat.get_value_or(nullptr));
    auto noiseCovDense = castMatrix::toDense(hMatrixNoiseCov.get_value_or(nullptr));
    TikhonovAlgorithmImpl algo(denseForward,
      measuredDense,
      gui_tikhonov_case,
      gui_tikhonov_solution_subcase,
      gui_tikhonov_residual_subcase,
      regMatDense,
      noiseCovDense,
      computeRegularizedInverse, this);

    TikhonovAlgorithmImpl::Input::lcurveGuiUpdate update = boost::bind(&SolveInverseProblemWithTikhonov::update_lcurve_gui, this, _1, _2, _3);

    TikhonovAlgorithmImpl::Input input(
      state->getValue(RegularizationMethod).toString(),
      state->getValue(LambdaFromDirectEntry).toDouble(),
      state->getValue(LambdaSliderValue).toDouble(),
      state->getValue(LambdaNum).toInt(),
      state->getValue(LambdaMin).toDouble(),
      state->getValue(LambdaMax).toDouble(),
      update);


    algo.run(input);

    if (computeRegularizedInverse)
    {
      sendOutput(RegInverse, algo.get_inverse_matrix());
    }

    sendOutput(InverseSolution, algo.get_inverse_solution());

    sendOutput(RegularizationParameter, algo.get_regularization_parameter());
  }
}

void SolveInverseProblemWithTikhonov::update_lcurve_gui(const double lambda, const BioPSE::TikhonovAlgorithm::LCurveInput& input, const int lambda_index)
{
  auto state = get_state();
  state->setValue(LambdaCorner, lambda);
  //estimate L curve corner
  const double lower_y = std::min(input.eta_[0] / 10.0, input.eta_[input.nLambda_ - 1]);

  std::ostringstream str;
  str << get_id() << " plot_graph \" ";
  for (int i = 0; i < input.nLambda_; i++)
    str << log10(input.rho_[i]) << " " << log10(input.eta_[i]) << " ";
  str << "\" \" " << log10(input.rho_[0] / 10.0) << " " << log10(input.eta_[lambda_index]) << " ";
  str << log10(input.rho_[lambda_index]) << " " << log10(input.eta_[lambda_index]) << " ";
  str << log10(input.rho_[lambda_index]) << " " << log10(lower_y) << " \" ";
  str << lambda << " " << lambda_index << " ; \n";

  state->setValue(LCurveText, str.str());
}

const AlgorithmParameterName SolveInverseProblemWithTikhonov::LambdaFromDirectEntry("LambdaFromDirectEntry");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::RegularizationMethod("RegularizationMethod");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::LambdaMin("LambdaMin");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::LambdaMax("LambdaMax");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::LambdaNum("LambdaNum");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::LambdaResolution("LambdaResolution");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::TikhonovCase("TikhonovCase");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::LambdaSliderValue("LambdaSliderValue");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::LambdaCorner("LambdaCorner");
const AlgorithmParameterName SolveInverseProblemWithTikhonov::LCurveText("LCurveText");