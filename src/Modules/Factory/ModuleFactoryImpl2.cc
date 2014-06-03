/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Modules/Factory/ModuleDescriptionLookup.h>
#include <Modules/Fields/InterfaceWithCleaver.h>
#include <Modules/Legacy/Fields/MapFieldDataFromElemToNode.h>
#include <Modules/Legacy/Fields/MapFieldDataFromNodeToElem.h>
#include <Modules/Legacy/Fields/CalculateVectorMagnitudes.h>
#include <Modules/Legacy/Fields/CalculateGradients.h>
#include <Modules/Legacy/Fields/GetFieldData.h>
#include <Modules/Legacy/Fields/SetFieldData.h>
#include <Modules/Legacy/Fields/ApplyMappingMatrix.h>
#include <Modules/Legacy/Math/SelectSubMatrix.h>
#include <Modules/Legacy/Math/ConvertMatrixType.h>
#include <Modules/FiniteElements/TDCSSimulator.h>
#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
#include <Modules/BrainStimulator/SetConductivitiesToTetMesh.h>
#include <Modules/BrainStimulator/SetupRHSforTDCSandTMS.h>
#include <Modules/BrainStimulator/GenerateROIStatistics.h>
#include <Modules/Legacy/Math/AddKnownsToLinearSystem.h>
#include <Modules/Legacy/FiniteElements/BuildTDCSMatrix.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Modules::BrainStimulator;

void ModuleDescriptionLookup::addBrainSpecificModules()
{
  addModuleDesc<BuildTDCSMatrix>("BuildTDCSMatrix", "FiniteElements", "SCIRun", " in progress ", "Generates tDCS Forward Matrix ");
  addModuleDesc<ElectrodeCoilSetupModule>("ElectrodeCoilSetup", "BrainStimulator", "SCIRun", " in progress ", " Place tDCS electrodes and TMS coils ");
  addModuleDesc<SetConductivitiesToTetMeshModule>("SetConductivitiesToTetMesh", "BrainStimulator", "SCIRun", " in progress ", " Sets conveniently conductivity profile for tetrahedral mesh ");
  addModuleDesc<GenerateROIStatisticsModule>("GenerateROIStatistics", "BrainStimulator", "SCIRun", " in progress ", " Roi statistics ");   
  addModuleDesc<SetupRHSforTDCSandTMSModule>("SetupRHSforTDCSandTMS", "BrainStimulator", "SCIRun", " in progress ", " set RHS for tDCS and TMS ");        
}

void ModuleDescriptionLookup::addMoreModules()
{
  addModuleDesc<AddKnownsToLinearSystem>("AddKnownsToLinearSystem", "Math", "SCIRun", " in progress ", " adds knowns to linear systems ");        
  addModuleDesc<CalculateVectorMagnitudesModule>("CalculateVectorMagnitudes", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<CalculateGradients >("CalculateGradients", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<GetFieldDataModule>("GetFieldData", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<InterfaceWithCleaverModule>("InterfaceWithCleaver", "NewField", "SCIRun", "New Module to interact with cleaver", "...");
  addModuleDesc<SetFieldDataModule>("SetFieldData", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<SelectSubMatrixModule>("SelectSubMatrix", "Math", "SCIRun", "in progress", "...");
  addModuleDesc<MapFieldDataFromElemToNodeModule>("MapFieldDataFromElemToNode", "ChangeFieldData", "SCIRun", "in progress", "...");
  addModuleDesc<ApplyMappingMatrixModule>("ApplyMappingMatrix", "ChangeFieldData", "SCIRun", "in progress", "...");
  addModuleDesc<ConvertMatrixTypeModule>("ConvertMatrixType", "Math", "SCIRun", "in progress", "...");
  addModuleDesc<MapFieldDataFromNodeToElemModule>("MapFieldDataFromNodeToElem", "ChangeFieldData", "SCIRun", "in progress", "...");
}