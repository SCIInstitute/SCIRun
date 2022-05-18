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
*/


///  File       : SolveInverseProblemWithTSVD.cc
///  Author     : Jaume Coll-Font, Moritz Dannhauer, Ayla Khan, Dan White
///  Date       : September 06th, 2017 (last update)

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTSVD.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Modules/Legacy/Inverse/LCurvePlot.h>
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithTSVD_impl.h>
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
MODULE_INFO_DEF(SolveInverseProblemWithTSVD, Inverse, SCIRun)

// Constructor neeeds to have empty inputs and the parent's constructor has staticInfo_ as an input (adapted to thsi module in MODULE_INFO_DEF macro)
// Constructor needs to initialize all input/output ports
SolveInverseProblemWithTSVD::SolveInverseProblemWithTSVD() : Module(staticInfo_)
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

void SolveInverseProblemWithTSVD::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Parameters::LambdaMin,1);
  state->setValue(Parameters::LambdaMax,100);
  state->setValue(Parameters::LambdaNum,100);
  state->setValue(Parameters::LambdaResolution,1);

  setStateStringFromAlgo(Parameters::TikhonovImplementation);
  setStateStringFromAlgoOption(Parameters::RegularizationMethod);
  setStateDoubleFromAlgo(Parameters::LambdaFromDirectEntry);
  setStateDoubleFromAlgo(Parameters::LambdaSliderValue);
}

void SolveInverseProblemWithTSVD::execute()
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

		// set parameters
		auto state = get_state();

		state->setValue( Parameters::TikhonovImplementation, std::string("TSVD") );
		setAlgoStringFromState(Parameters::TikhonovImplementation);
		setAlgoOptionFromState(Parameters::RegularizationMethod);
		setAlgoDoubleFromState(Parameters::LambdaFromDirectEntry);

    int LambdaMin= std::max(static_cast <int> (state->getValue(Parameters::LambdaMin).toDouble()),1);
    int LambdaMax= std::min(static_cast <int> (state->getValue(Parameters::LambdaMax).toDouble()),rank);

    if (LambdaMax<LambdaMin)
    {
      int tmp = LambdaMax;
      LambdaMax=LambdaMin;
      LambdaMin=tmp;
    }

    int LambdaNum= LambdaMax - LambdaMin+1;

    state->setValue(Parameters::LambdaNum, LambdaNum);
    state->setValue(Parameters::LambdaMin, static_cast <double> (LambdaMin));
    state->setValue(Parameters::LambdaMax, static_cast <double> (LambdaMax));

		setAlgoDoubleFromState(Parameters::LambdaMin);
		setAlgoDoubleFromState(Parameters::LambdaMax);
		setAlgoIntFromState(Parameters::LambdaNum);
		setAlgoDoubleFromState(Parameters::LambdaSliderValue);

		// run
		auto output = algo().run(
							withInputData(
								(ForwardMatrix, forward_matrix_h)
								(MeasuredPotentials,hMatrixMeasDat)
								(MeasuredPotentials,hMatrixMeasDat)
								(WeightingInSourceSpace,optionalAlgoInput(hMatrixRegMat))
								(WeightingInSensorSpace,optionalAlgoInput(hMatrixNoiseCov))
								(matrixU,optionalAlgoInput(hMatrixU))(singularValues,optionalAlgoInput(hSingularValues))
								(matrixV,optionalAlgoInput(hMatrixV)))
							);


		// set outputs
		sendOutputFromAlgorithm(InverseSolution,output);
		sendOutputFromAlgorithm(RegularizationParameter,output);
		sendOutputFromAlgorithm(RegInverse,output);
    auto lambda=output.get<DenseMatrix>(TikhonovAlgoAbstractBase::RegularizationParameter);
    auto lambda_array=output.get<DenseMatrix>(TikhonovAlgoAbstractBase::LambdaArray);
    auto lambda_index =output.get<DenseMatrix>(TikhonovAlgoAbstractBase::Lambda_Index);

    auto regularization_method  = state->getValue(Parameters::RegularizationMethod).toString();

    if (regularization_method== "lcurve")
    {
			LCurvePlot helper;
      auto str = helper.update_lcurve_gui(id(),lambda,lambda_array,lambda_index);
      state->setTransientValue("LambdaCorner", lambda->get(0,0));
      state->setTransientValue("LambdaCurveInfo", str);
      state->setTransientValue("LambdaCurve", lambda_array);
      state->setTransientValue("LambdaCornerPlot", helper.cornerPlot());
    }
	}
}
