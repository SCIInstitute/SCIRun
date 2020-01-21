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


/// @todo Documentation Modules/Legacy/Fields/RefineTetMeshLocally.cc

#include <Core/Algorithms/Field/RefineTetMeshLocallyAlgorithm.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Fields/RefineTetMeshLocally.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

MODULE_INFO_DEF(RefineTetMeshLocally, ChangeMesh, SCIRun)

RefineTetMeshLocally::RefineTetMeshLocally()
		: Module(staticInfo_, true)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void RefineTetMeshLocally::setStateDefaults()
{
 setStateDoubleFromAlgo(Parameters::RefineTetMeshLocallyIsoValue);
 setStateDoubleFromAlgo(Parameters::RefineTetMeshLocallyEdgeLength);
 setStateDoubleFromAlgo(Parameters::RefineTetMeshLocallyVolume);
 setStateDoubleFromAlgo(Parameters::RefineTetMeshLocallyDihedralAngleSmaller);
 setStateDoubleFromAlgo(Parameters::RefineTetMeshLocallyDihedralAngleBigger);
 setStateIntFromAlgo(Parameters::RefineTetMeshLocallyMaxNumberRefinementIterations);
 setStateIntFromAlgo(Parameters::RefineTetMeshLocallyRadioButtons);
 setStateBoolFromAlgo(Parameters::RefineTetMeshLocallyDoNoSplitSurfaceTets);
 setStateBoolFromAlgo(Parameters::RefineTetMeshLocallyCounterClockWiseOrdering);
 setStateBoolFromAlgo(Parameters::RefineTetMeshLocallyUseModuleInputField);
}

void RefineTetMeshLocally::execute()
{
  auto inputfield_ = getRequiredInput(InputField);

  if (needToExecute())
  {
    setAlgoDoubleFromState(Parameters::RefineTetMeshLocallyIsoValue);
    setAlgoDoubleFromState(Parameters::RefineTetMeshLocallyEdgeLength);
    setAlgoDoubleFromState(Parameters::RefineTetMeshLocallyVolume);
    setAlgoDoubleFromState(Parameters::RefineTetMeshLocallyDihedralAngleSmaller);
    setAlgoDoubleFromState(Parameters::RefineTetMeshLocallyDihedralAngleBigger);
    setAlgoBoolFromState(Parameters::RefineTetMeshLocallyDoNoSplitSurfaceTets);
    setAlgoBoolFromState(Parameters::RefineTetMeshLocallyCounterClockWiseOrdering);
    setAlgoBoolFromState(Parameters::RefineTetMeshLocallyUseModuleInputField);
    setAlgoIntFromState(Parameters::RefineTetMeshLocallyMaxNumberRefinementIterations);
    auto RadioButton = (get_state()->getValue(Parameters::RefineTetMeshLocallyRadioButtons)).toInt();
    if (RadioButton == -1)
      RadioButton = 0;
    algo().set(Parameters::RefineTetMeshLocallyRadioButtons, RadioButton);
    auto output = algo().run(withInputData((InputField, inputfield_)));
    sendOutputFromAlgorithm(OutputField, output);
  }
}
