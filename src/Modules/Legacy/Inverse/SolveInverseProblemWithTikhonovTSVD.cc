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
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonovTSVD.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithTikhonovTSVD_impl.h>
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
MODULE_INFO_DEF(SolveInverseProblemWithTikhonovTSVD, Inverse, SCIRun)

// Constructor neeeds to have empty inputs and the parent's constructor has staticInfo_ as an input (adapted to thsi module in MODULE_INFO_DEF macro)
// Constructor needs to initialize all input/output ports
SolveInverseProblemWithTikhonovTSVD::SolveInverseProblemWithTikhonovTSVD() : Module(staticInfo_)
{
	//inputs
	INITIALIZE_PORT(ForwardMatrix);
	INITIALIZE_PORT(WeightingInSourceSpace);
	INITIALIZE_PORT(MeasuredPotentials);
	INITIALIZE_PORT(WeightingInSensorSpace);
	INITIALIZE_PORT(matrixU);
	INITIALIZE_PORT(singularValues);
	INITIALIZE_PORT(matrixV);
	// outputs
	INITIALIZE_PORT(InverseSolution);
	INITIALIZE_PORT(RegularizationParameter);
	INITIALIZE_PORT(RegInverse);
}

void SolveInverseProblemWithTikhonovTSVD::setStateDefaults()
{
	auto state = get_state();

	state->setValue( Parameters::TikhonovImplementation, std::string("TikhonovTSVD") );
	state->setValue(Parameters::LambdaMin,1);
	state->setValue(Parameters::LambdaMax,1);
	state->setValue(Parameters::LambdaNum, 1);
	state->setValue( Parameters::LambdaResolution, 1);

	// setStateStringFromAlgo(Parameters::TikhonovImplementation);
	setStateStringFromAlgoOption(Parameters::RegularizationMethod);
	setStateDoubleFromAlgo(Parameters::LambdaFromDirectEntry);
	// setStateDoubleFromAlgo(Parameters::LambdaMin);
	// setStateDoubleFromAlgo(Parameters::LambdaMax);
	// setStateIntFromAlgo(Parameters::LambdaNum);
	// setStateDoubleFromAlgo(Parameters::LambdaResolution);
	setStateDoubleFromAlgo(Parameters::LambdaSliderValue);
	setStateIntFromAlgo(Parameters::LambdaCorner);
	setStateStringFromAlgo(Parameters::LCurveText);
}

// execute function
void SolveInverseProblemWithTikhonovTSVD::execute()
{

	// load required inputs
	auto forward_matrix_h = getRequiredInput(ForwardMatrix);
	auto hMatrixMeasDat = getRequiredInput(MeasuredPotentials);

	// load regularization optional inputs
	auto hMatrixRegMat = getOptionalInput(WeightingInSourceSpace);
	auto hMatrixNoiseCov = getOptionalInput(WeightingInSensorSpace);

	// load SVD optional inputs
	auto hMatrixU = getOptionalInput(matrixU);
	auto hSingularValues = getOptionalInput(singularValues);
	auto hMatrixV = getOptionalInput(matrixV);


	if (needToExecute())
	{

		// Obtain rank of forward matrix
		int rank;
		if ( hSingularValues ) {
			rank = (*hSingularValues)->nrows();
		}
		else{
			Eigen::FullPivLU<SCIRun::Core::Datatypes::DenseMatrix::EigenBase> lu_decomp(*forward_matrix_h);
			rank = lu_decomp.rank();
		}

		std::cout << "Computed rank: " << rank << std::endl;
		// set parameters
		auto state = get_state();

		state->setValue( Parameters::TikhonovImplementation, std::string("TikhonovTSVD") );
		setAlgoStringFromState(Parameters::TikhonovImplementation);
		setAlgoOptionFromState(Parameters::RegularizationMethod);
		setAlgoDoubleFromState(Parameters::LambdaFromDirectEntry);
		// state->setValue(Parameters::LambdaMin,1);
		state->setValue(Parameters::LambdaMax,double(rank)); // casting to double to keep consistency across tikhonov types
		state->setValue(Parameters::LambdaNum, rank);	// casting to double to keep consistency across tikhonov types
		// state->setValue( Parameters::LambdaResolution, 1);
		setAlgoDoubleFromState(Parameters::LambdaMin);
		setAlgoDoubleFromState(Parameters::LambdaMax);
		setAlgoIntFromState(Parameters::LambdaNum);
		setAlgoDoubleFromState(Parameters::LambdaSliderValue);
		setAlgoIntFromState(Parameters::LambdaCorner);
		setAlgoStringFromState(Parameters::LCurveText);

		// run
		auto output = algo().run( withInputData((ForwardMatrix, forward_matrix_h)(MeasuredPotentials,hMatrixMeasDat)(MeasuredPotentials,hMatrixMeasDat)(WeightingInSourceSpace,optionalAlgoInput(hMatrixRegMat))(WeightingInSensorSpace,optionalAlgoInput(hMatrixNoiseCov))(matrixU,optionalAlgoInput(hMatrixU))(singularValues,optionalAlgoInput(hSingularValues))(matrixV,optionalAlgoInput(hMatrixV))) );

		// update L-curve
		/* NO EXISTE
        SolveInverseProblemWithTikhonovTSVD_impl::Input::lcurveGuiUpdate update = boost::bind(&SolveInverseProblemWithTikhonov::update_lcurve_gui, this, _1, _2, _3);
		*/

		// set outputs
		sendOutputFromAlgorithm(InverseSolution,output);
		sendOutputFromAlgorithm(RegularizationParameter,output);
		sendOutputFromAlgorithm(RegInverse,output);

	}
}
