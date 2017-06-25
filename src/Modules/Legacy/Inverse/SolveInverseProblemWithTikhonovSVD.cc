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
*/

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithTikhonovSVD_impl.h>
// #include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Inverse;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Inverse;

// Module definitions. Sets the info into the staticInfo_
MODULE_INFO_DEF(SolveInverseProblemWithTikhonovSVD, Inverse, SCIRun)

// Constructor neeeds to have empty inputs and the parent's constructor has staticInfo_ as an input (adapted to thsi module in MODULE_INFO_DEF macro)
// Constructor needs to initialize all input/output ports
SolveInverseProblemWithTikhonovSVD::SolveInverseProblemWithTikhonovSVD() : Module(staticInfo_)
{
	//inputs
	INITIALIZE_PORT(ForwardMatrix);
	INITIALIZE_PORT(WeightingInSourceSpace);
	INITIALIZE_PORT(MeasuredPotentials);
	INITIALIZE_PORT(WeightingInSensorSpace);
	// outputs
	INITIALIZE_PORT(InverseSolution);
	INITIALIZE_PORT(RegularizationParameter);
	INITIALIZE_PORT(RegInverse);
}

void SolveInverseProblemWithTikhonovSVD::setStateDefaults()
{
	setStateStringFromAlgo(Parameters::TikhonovImplementation);
	setStateStringFromAlgoOption(Parameters::RegularizationMethod);
	setStateIntFromAlgo(Parameters::regularizationChoice);
	setStateDoubleFromAlgo(Parameters::LambdaFromDirectEntry);
	setStateDoubleFromAlgo(Parameters::LambdaMin);
	setStateDoubleFromAlgo(Parameters::LambdaMax);
	setStateIntFromAlgo(Parameters::LambdaNum);
	setStateDoubleFromAlgo(Parameters::LambdaResolution);
	setStateDoubleFromAlgo(Parameters::LambdaSliderValue);
	setStateIntFromAlgo(Parameters::LambdaCorner);
	setStateStringFromAlgo(Parameters::LCurveText);
	setStateIntFromAlgo(Parameters::regularizationSolutionSubcase);
	setStateIntFromAlgo(Parameters::regularizationResidualSubcase);
}

// execute function
void SolveInverseProblemWithTikhonovSVD::execute()
{

	// load required inputs
	auto forward_matrix_h = getRequiredInput(ForwardMatrix);
	auto hMatrixMeasDat = getRequiredInput(MeasuredPotentials);

	// load optional inputs
	auto hMatrixRegMat = getOptionalInput(WeightingInSourceSpace);
	auto hMatrixNoiseCov = getOptionalInput(WeightingInSensorSpace);

	std::cout << "gato" << std::endl;

	if (needToExecute())
	{
		// set parameters
		auto state = get_state();
		// set parameters
		state->setValue( Parameters::TikhonovImplementation, std::string("TikhonovSVD") );
		setAlgoStringFromState(Parameters::TikhonovImplementation);
		setAlgoOptionFromState(Parameters::RegularizationMethod);
		setAlgoIntFromState(Parameters::regularizationChoice);
		setAlgoDoubleFromState(Parameters::LambdaFromDirectEntry);
		setAlgoDoubleFromState(Parameters::LambdaMin);
		setAlgoDoubleFromState(Parameters::LambdaMax);
		setAlgoIntFromState(Parameters::LambdaNum);
		setAlgoDoubleFromState(Parameters::LambdaResolution);
		setAlgoDoubleFromState(Parameters::LambdaSliderValue);
		setAlgoIntFromState(Parameters::LambdaCorner);
		setAlgoStringFromState(Parameters::LCurveText);
		setAlgoIntFromState(Parameters::regularizationSolutionSubcase);
		setAlgoIntFromState(Parameters::regularizationResidualSubcase);

		// run
		// run
		auto output = algo().run( withInputData((ForwardMatrix, forward_matrix_h)(MeasuredPotentials,hMatrixMeasDat)(MeasuredPotentials,hMatrixMeasDat)(WeightingInSourceSpace,optionalAlgoInput(hMatrixRegMat))(WeightingInSensorSpace,optionalAlgoInput(hMatrixNoiseCov))) );

		// update L-curve
		/* NO EXISTE
        SolveInverseProblemWithTikhonovSVD_impl::Input::lcurveGuiUpdate update = boost::bind(&SolveInverseProblemWithTikhonov::update_lcurve_gui, this, _1, _2, _3);
		*/

		// set outputs
		sendOutputFromAlgorithm(InverseSolution,output);
		sendOutputFromAlgorithm(RegularizationParameter,output);
		sendOutputFromAlgorithm(RegInverse,output);

	}
}
