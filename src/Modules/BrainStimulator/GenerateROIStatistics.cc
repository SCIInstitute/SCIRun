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
#include <iostream>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/BrainStimulator/GenerateROIStatistics.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

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
  setStateStringFromAlgoOption(Parameters::PhysicalUnitStr);  
}

void GenerateROIStatisticsModule::execute()
{
  auto meshData_ = getRequiredInput(MeshDataOnElements); /// get all the inputs
  auto physicalUnit_ = getOptionalInput(PhysicalUnit);
  auto atlasMesh_ = getRequiredInput(AtlasMesh);
  auto atlasMeshLabels_ = getOptionalInput(AtlasMeshLabels);
  auto coordinateSpace_ = getOptionalInput(CoordinateSpace);
  auto coordinateSpaceLabel_ = getOptionalInput(CoordinateSpaceLabel);
  
  setAlgoListFromState(Parameters::StatisticsTableValues); /// to transfer data between algo and dialog use the state variable 
  
  auto roiSpec = transient_value_cast<DenseMatrixHandle>(get_state()->getTransientValue(GenerateROIStatisticsAlgorithm::SpecifyROI)); /// transfer the ROI specification from GUI dialog as additional input
  //algorithm input and run
  auto input = make_input((MeshDataOnElements, meshData_)(PhysicalUnit, optionalAlgoInput(physicalUnit_))(AtlasMesh, atlasMesh_)(AtlasMeshLabels,
  optionalAlgoInput(atlasMeshLabels_))(CoordinateSpace, optionalAlgoInput(coordinateSpace_))(CoordinateSpaceLabel,
  optionalAlgoInput(coordinateSpaceLabel_)));
  
  if (roiSpec)
    input[GenerateROIStatisticsAlgorithm::SpecifyROI] = roiSpec;
  
  auto output = algo().run_generic(input); /// call run generic 

  auto table = output.additionalAlgoOutput(); /// get the two outputs, the upper table (as DenseMatrix) and the container that establishes data transfer between GUI/Algo via state  
  get_state()->setTransientValue(Parameters::StatisticsTableValues, table);
  
  if (physicalUnit_ && *physicalUnit_) /// set physicalUnit string immediately to state, and then to dialog
     get_state()->setTransientValue(Parameters::PhysicalUnitStr,(*physicalUnit_)->value());
  
  if (coordinateSpaceLabel_ && *coordinateSpaceLabel_)/// set coordinateSpaceLabel string immediately to state, and then to dialog
     get_state()->setTransientValue(Parameters::CoordinateSpaceLabelStr,(*coordinateSpaceLabel_)->value()); 
  
  //algorithm output
  sendOutputFromAlgorithm(StatisticalResults, output); /// set the upper table (DenseMatrix) as only output of the module
}
