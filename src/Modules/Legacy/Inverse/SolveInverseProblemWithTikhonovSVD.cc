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


#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD_impl.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovImplAbstractBase.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>


using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Inverse;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Inverse;

/////////////////////////////////////////
// MODULE EXECUTION
/////////////////////////////////////////
void SolveInverseProblemWithTikhonovSVD::execute()
{
        auto forward_matrix_h = getRequiredInput(ForwardMatrix);
        auto hMatrixMeasDat = getRequiredInput(MeasuredPotentials);


        const bool computeRegularizedInverse = oport_connected(InverseSolution);

        if (needToExecute())
        {

            using namespace BioPSE;
            auto state = get_state();


            auto denseForward = castMatrix::toDense(forward_matrix_h);
            auto measuredDense = convertMatrix::toDense(hMatrixMeasDat);


            SolveInverseProblemWithTikhonovSVD_impl algo(   denseForward,
                                                            measuredDense,
                                                            computeRegularizedInverse,
                                                            this);

            TikhonovImplAbstractBase::Input::lcurveGuiUpdate update = boost::bind(&SolveInverseProblemWithTikhonovSVD::update_lcurve_gui, this, _1, _2, _3);

            TikhonovImplAbstractBase::Input input(
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

///////////////////////////////////////
//// Constructor
///////////////////////////////
SolveInverseProblemWithTikhonovSVD::SolveInverseProblemWithTikhonovSVD() : Module(staticInfo_)
{
    INITIALIZE_PORT(ForwardMatrix);
    INITIALIZE_PORT(WeightingInSourceSpace);
    INITIALIZE_PORT(MeasuredPotentials);
    INITIALIZE_PORT(WeightingInSensorSpace);
    INITIALIZE_PORT(InverseSolution);
    INITIALIZE_PORT(RegularizationParameter);
    INITIALIZE_PORT(RegInverse);
}


///////////////////////////////////////
/// Set DEFAULTS
//////////////////////////////////////
void SolveInverseProblemWithTikhonovSVD::setStateDefaults()
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

/////////////////////////////////////
////// MODULE UI DECLARATION
///////////////////////////////////
const ModuleLookupInfo SolveInverseProblemWithTikhonovSVD::staticInfo_("SolveInverseProblemWithTikhonovSVD", "Inverse", "SCIRun");


///////////////////////////////////
//// UPDATE L-CURVE IN GUI
//////////////////////////////////
void SolveInverseProblemWithTikhonovSVD::update_lcurve_gui(const double lambda, const BioPSE::TikhonovAlgorithm::LCurveInput& input, const int lambda_index)
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


///////////////////////////////////////
//// DEFINE PARAMETERS
/////////////////////////////////
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LambdaFromDirectEntry("LambdaFromDirectEntry");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::RegularizationMethod("RegularizationMethod");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LambdaMin("LambdaMin");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LambdaMax("LambdaMax");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LambdaNum("LambdaNum");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LambdaResolution("LambdaResolution");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::TikhonovCase("TikhonovCase");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LambdaSliderValue("LambdaSliderValue");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LambdaCorner("LambdaCorner");
const AlgorithmParameterName SolveInverseProblemWithTikhonovSVD::LCurveText("LCurveText");
