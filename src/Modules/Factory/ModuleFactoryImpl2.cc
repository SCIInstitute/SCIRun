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


#include <Modules/Factory/ModuleDescriptionLookup.h>
#include <Modules/Fields/InterfaceWithCleaver.h>
#include <Modules/Fields/RefineTetMeshLocally.h>
#include <Modules/Legacy/Fields/MapFieldDataFromElemToNode.h>
#include <Modules/Legacy/Fields/MapFieldDataFromNodeToElem.h>
#include <Modules/Legacy/Fields/CalculateVectorMagnitudes.h>
#include <Modules/Legacy/Fields/ConvertFieldBasis.h>
#include <Modules/Legacy/Fields/GetFieldData.h>
#include <Modules/Legacy/Fields/SetFieldData.h>
#include <Modules/Legacy/Fields/ApplyMappingMatrix.h>
#include <Modules/Legacy/Fields/SplitFieldByConnectedRegion.h>
#include <Modules/Legacy/Math/SelectSubMatrix.h>
#include <Modules/Legacy/Math/ConvertMatrixType.h>
#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
#include <Modules/BrainStimulator/SetConductivitiesToTetMesh.h>
#include <Modules/BrainStimulator/SetupRHSforTDCSandTMS.h>
#include <Modules/BrainStimulator/GenerateROIStatistics.h>
#include <Modules/BrainStimulator/SimulateForwardMagneticField.h>
#include <Modules/Legacy/Math/AddKnownsToLinearSystem.h>
#include <Modules/Legacy/FiniteElements/BuildTDCSMatrix.h>
#include <Modules/Legacy/FiniteElements/BuildFEVolRHS.h>
#include <Modules/Legacy/Visualization/GenerateStreamLines.h>
#include <Modules/Legacy/Inverse/BuildSurfaceLaplacianMatrix.h>
#include <Modules/Legacy/Fields/ConvertHexVolToTetVol.h>
#include <Modules/Legacy/Fields/ExtractSimpleIsosurface.h>
#include <Modules/Legacy/Fields/ClipVolumeByIsovalue.h>
#include <Modules/Math/ComputePCA.h>
#include <Modules/Visualization/ShowString.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Modules::Inverse;

void ModuleDescriptionLookup::addBrainSpecificModules()
{
  addModuleDesc<ElectrodeCoilSetup>("ElectrodeCoilSetup", "BrainStimulator", "SCIRun", " in progress ", " Place tDCS electrodes and TMS coils ");
  addModuleDesc<SetConductivitiesToMesh>("SetConductivitiesToMesh", "BrainStimulator", "SCIRun", "New module", " Sets conveniently conductivity profile for tetrahedral mesh ");
  addModuleDesc<GenerateROIStatistics>("GenerateROIStatistics", "BrainStimulator", "SCIRun", " in progress ", " Roi statistics ");
  addModuleDesc<SetupTDCS>("SetupTDCS", "BrainStimulator", "SCIRun", " in progress ", " set RHS for tDCS and TMS ");
}

void ModuleDescriptionLookup::addMoreModules()
{
  addModuleDesc<AddKnownsToLinearSystem>("AddKnownsToLinearSystem", "Math", "SCIRun", "Real ported module", "...");
  addModuleDesc<CalculateVectorMagnitudes>("CalculateVectorMagnitudes", "ChangeFieldData", "SCIRun", "Real ported module", "...");
	addModuleDesc<ConvertFieldBasis>("ConvertFieldBasis", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<GetFieldData>("GetFieldData", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<InterfaceWithCleaver>("InterfaceWithCleaver", "NewField", "SCIRun", "New module", "...");
  addModuleDesc<SetFieldData>("SetFieldData", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<SelectSubMatrix>("SelectSubMatrix", "Math", "SCIRun", "Real ported module", "...");
  addModuleDesc<MapFieldDataFromElemToNode>("MapFieldDataFromElemToNode", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<ApplyMappingMatrix>("ApplyMappingMatrix", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<ConvertMatrixType>("ConvertMatrixType", "Math", "SCIRun", "Real ported module", "...");
  addModuleDesc<MapFieldDataFromNodeToElem>("MapFieldDataFromNodeToElem", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<BuildFEVolRHS>("BuildFEVolRHS", "FiniteElements", "SCIRun", "Real ported module", "...");
  addModuleDesc<GenerateStreamLines>("in progress--needs testing", "...");
  addModuleDesc<ConvertHexVolToTetVol>("ConvertHexVolToTetVol", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<BuildSurfaceLaplacianMatrix>("BuildSurfaceLaplacianMatrix","Inverse","SCIRun","...","...");
  addModuleDesc<ExtractIsosurface>("...","...");
  addModuleDesc<ExtractIsosurface>("ExtractSimpleIsosurface", "NewField", "SCIRun", "...", "...");
  addModuleDesc<ComputePCA>("ComputePCA","Math","SCIRun","...","...");
  addModuleDesc<ClipVolumeByIsovalue>("ClipVolumeByIsovalue","NewField","SCIRun","...","...");
  addModuleDesc<RefineTetMeshLocally>("RefineTetMeshLocally","ChangeMesh","SCIRun","...","...");
  addModuleDesc<ShowString>("in progress--needs testing", "...");
}
