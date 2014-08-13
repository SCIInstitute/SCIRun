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
#include <iostream>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/BrainStimulator/GenerateROIStatistics.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Dataflow::Networks;

GenerateROIStatisticsModule::GenerateROIStatisticsModule() : Module(ModuleLookupInfo("GenerateROIStatistics", "BrainStimulator", "SCIRun"))
{
 INITIALIZE_PORT(MeshDataOnElements);
 INITIALIZE_PORT(PhysicalUnit);
 INITIALIZE_PORT(AtlasMesh);
 INITIALIZE_PORT(AtlasMeshLabels);
 INITIALIZE_PORT(CoordinateSpace);
 INITIALIZE_PORT(StatisticalResults);
 INITIALIZE_PORT(CoordinateSpaceLabel);
}

void GenerateROIStatisticsModule::setStateDefaults()
{
  //setStateListFromAlgo(Parameters::StatisticsTableValues);  
}

void GenerateROIStatisticsModule::execute()
{
  auto meshData_ = getRequiredInput(MeshDataOnElements);
  auto physicalUnit_ = getOptionalInput(PhysicalUnit);
  auto atlasMesh_ = getRequiredInput(AtlasMesh);
  auto atlasMeshLabels_ = getOptionalInput(AtlasMeshLabels);
  auto coordinateSpace_ = getOptionalInput(CoordinateSpace);
  auto coordinateSpaceLabel_ = getOptionalInput(CoordinateSpace);
  
  setAlgoListFromState(Parameters::StatisticsTableValues);
  //auto elc_vals_from_state = get_state()->getValue(Parameters::ElectrodeTableValues).getList();
  //algo().set(Parameters::ELECTRODE_VALUES, elc_vals_from_state);
  
  //algorithm input and run
  auto output = algo().run_generic(make_input((MeshDataOnElements, meshData_)(PhysicalUnit, optionalAlgoInput(physicalUnit_))(AtlasMesh, atlasMesh_)(AtlasMeshLabels, optionalAlgoInput(atlasMeshLabels_))(CoordinateSpace, optionalAlgoInput(coordinateSpace_))(CoordinateSpaceLabel, optionalAlgoInput(coordinateSpaceLabel_))));

  auto table = output.additionalAlgoOutput();
  get_state()->setValue(Parameters::StatisticsTableValues, table.value_);

  //algorithm output
  sendOutputFromAlgorithm(StatisticalResults, output);
}
